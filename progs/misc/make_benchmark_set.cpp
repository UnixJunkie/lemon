#include <iostream>
#include <sstream>

#include "lemon/lemon.hpp"

int main(int argc, char* argv[]) {
    lemon::Options o(argc, argv);

    double distance = o.distance();
    auto entries = o.entries();

    if (!boost::filesystem::is_regular_file(entries)) {
        std::cerr << "You must supply a valid entries file" << std::endl;
        return 1;
    }

    auto p = o.work_dir();

    if (!boost::filesystem::is_directory(p)) {
        std::cerr << "You must supply a valid archive directory" << std::endl;
        return 2;
    }

    auto outdir = o.work_dir();
    auto threads = o.ncpu();

    lemon::PDBIDVec vec;
    std::unordered_map<std::string, lemon::ResidueNameSet> rnms;
    std::ifstream is(entries);
    lemon::read_entry_file(is, vec, rnms);

    auto worker = [distance, &rnms, &outdir](chemfiles::Frame complex,
                                             const std::string& pdbid) {

        // Selection phase
        auto smallm = lemon::select_specific_residues(complex, rnms[pdbid]);

        // Pruning phase
        lemon::remove_identical_residues(complex, smallm);

        // Output phase
        for (auto resid : smallm) {
            chemfiles::Frame prot;
            chemfiles::Frame lig;
            lemon::separate_protein_and_ligand(complex, resid, prot, lig,
                                               distance);

            auto protfile = boost::filesystem::path(outdir);
            protfile /= pdbid + "_" + lig.get("name")->as_string() + ".pdb";
            auto ligfile  = boost::filesystem::path(outdir);
            ligfile /= pdbid + "_" + lig.get("name")->as_string() + ".sdf";

            chemfiles::Trajectory prot_traj(protfile.string(),'w');
            chemfiles::Trajectory lig_traj(ligfile.string(),'w');
            prot_traj.write(prot);
            lig_traj.write(lig);
            prot_traj.close();
            lig_traj.close();
        }
    };

    try {
        lemon::run_parallel(worker, p, threads);
    } catch(std::runtime_error& e){
        std::cerr << e.what() << "\n";
        return 1;
    }
}
