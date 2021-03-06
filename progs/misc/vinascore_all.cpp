#include <iostream>
#include <sstream>
#include "lemon/lemon.hpp"
#include "lemon/launch.hpp"
#include "lemon/xscore.hpp"

int main(int argc, char* argv[]) {
    lemon::Options o(argc, argv);

    auto worker = [](const chemfiles::Frame& entry,
                     const std::string& pdbid) -> std::string {

        // Selection phase= 
        auto smallm = lemon::select::small_molecules(entry);
        if (smallm.empty()) {
            return std::string("");
        }

        // Pruning phase
        lemon::prune::identical_residues(entry, smallm);
        lemon::prune::cofactors(entry, smallm, lemon::common_cofactors);
        lemon::prune::cofactors(entry, smallm, lemon::common_fatty_acids);

        // Output phase
        const auto& residues = entry.topology().residues();
        std::list<size_t> proteins;
        for (size_t i = 0; i < entry.topology().residues().size(); ++i) {
            proteins.push_back(i);
        }

        std::string result;
        for (auto smallm_id : smallm) {
            auto prot_copy = proteins;

            lemon::prune::keep_interactions(entry, smallm, prot_copy, lemon::xscore::DEFAULT_INTERACTION_DISTANCE);
            prot_copy.erase(std::remove(prot_copy.begin(), prot_copy.end(), smallm_id), prot_copy.end());

            auto vscore =
                lemon::xscore::vina_score(entry, smallm_id, prot_copy);

            result += pdbid + "\t" +
                residues[smallm_id].name() + "\t" +
                std::to_string(vscore.g1) + "\t" +
                std::to_string(vscore.g2) + "\t" +
                std::to_string(vscore.hydrogen) + "\t" +
                std::to_string(vscore.hydrophobic) + "\t" +
                std::to_string(vscore.rep) + "\n";
        }

        return result;
    };

    auto collector = lemon::print_combine(std::cout);
    return lemon::launch(o, worker, collector);
}
