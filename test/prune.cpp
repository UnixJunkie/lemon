#include "lemon/prune.hpp"
#include <chemfiles.hpp>
#include "lemon/select.hpp"
#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Select and prune P30 from 4XUF") {
    auto traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = lemon::select::small_molecules(frame);
    CHECK(res.size() == 2);

    lemon::prune::identical_residues(frame, res);
    CHECK(res.size() == 1);
    std::cout << "HERE" << std::endl;
}

TEST_CASE("Select and prune 1PG/HEM from 1D7D") {
    auto traj = chemfiles::Trajectory("files/1D7D.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = lemon::select::small_molecules(frame);
    CHECK(res.size() == 3);  // Two hemes and 1PG

    lemon::prune::cofactors(frame, res, lemon::common_fatty_acids);
    CHECK(res.size() == 2);

    lemon::prune::cofactors(frame, res, lemon::common_cofactors);
    CHECK(res.size() == 0);
    std::cout << "HERE" << std::endl;
}

TEST_CASE("Remove non-nucleic acid interactions") {
    auto traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = lemon::select::small_molecules(frame);
    auto nas = lemon::select::nucleic_acids(frame);
    lemon::prune::keep_interactions(frame, res, nas);
    CHECK(res.size() == 0);  // It got removed

    traj = chemfiles::Trajectory("files/entry_10/1/0/100D.mmtf.gz", 'r');
    frame = traj.read();

    res = lemon::select::small_molecules(frame);
    nas = lemon::select::nucleic_acids(frame);
    lemon::prune::keep_interactions(frame, res, nas);
    CHECK(res.size() == 1);  // Not removed
    std::cout << "HERE" << std::endl;
}

TEST_CASE("Remove non-metal interactions") {
    auto traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = lemon::select::small_molecules(frame);
    auto metals = lemon::select::metal_ions(frame);
    lemon::prune::keep_interactions(frame, res, metals);
    CHECK(res.size() == 0);  // It got removed

    traj = chemfiles::Trajectory("files/1OQ5.mmtf.gz", 'r');
    frame = traj.read();

    res = lemon::select::small_molecules(frame);
    metals = lemon::select::metal_ions(frame);
    lemon::prune::keep_interactions(frame, res, metals);
    CHECK(res.size() == 1);  // Not removed
    std::cout << "HERE" << std::endl;
}

TEST_CASE("Remove ligands that interact with water") {
    auto traj = chemfiles::Trajectory("files/1AAQ.mmtf", 'r');
    auto frame = traj.read();

    auto res = lemon::select::small_molecules(frame);
    auto hoh = lemon::select::specific_residues(frame, {"HOH"});
    lemon::prune::remove_interactions(frame, res, hoh);
    CHECK(res.size() == 0);  // It got removed

    traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    frame = traj.read();

    res = lemon::select::small_molecules(frame);
    hoh = lemon::select::specific_residues(frame, {"HOH"});
    lemon::prune::remove_interactions(frame, res, hoh);
    CHECK(res.size() == 2);  // Not removed
    std::cout << "HERE" << std::endl;
}

TEST_CASE("Intersection of chain ID and peptides") {
    auto traj = chemfiles::Trajectory("files/1AAQ.mmtf", 'r');
    auto frame = traj.read();

    auto res = lemon::select::peptides(frame);
    CHECK(res.size() == 198);

    auto chA = lemon::select::residue_property(frame, "chainid", "A");
    lemon::prune::intersection(res, chA);
    CHECK(res.size() == 99);

    auto res2 = lemon::select::peptides(frame);
    CHECK(res2.size() == 198);
    lemon::prune::has_property(frame, res2, "chainid", "A");
    CHECK(res2.size() == 99);
    std::cout << "HERE" << std::endl;
}