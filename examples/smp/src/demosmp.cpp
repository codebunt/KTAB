// --------------------------------------------
// Copyright KAPSARC. Open source MIT License.
// --------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2015 King Abdullah Petroleum Studies and Research Center
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// --------------------------------------------
//
// Demonstrate a very basic, but highly parameterized, Spatial Model of Politics.
//
// --------------------------------------------


#include "kutils.h"
#include "kmodel.h"
#include "smp.h"
#include "demosmp.h"
#include "csv_parser.hpp"


using KBase::PRNG;
using KBase::KMatrix;
using KBase::Actor;
using KBase::Model;
using KBase::Position;
using KBase::State;
using KBase::VotingRule;


namespace DemoSMP {
  
using std::cout;
using std::endl;
using std::flush;
using std::function;
using std::get;
using std::string;

using KBase::ReportingLevel;

using SMPLib::SMPModel;
using SMPLib::SMPActor;
using SMPLib::SMPState;

// -------------------------------------------------
void demoActorUtils(uint64_t s, PRNG* rng) {

    printf("Using PRNG seed: %020lu \n", s);
    rng->setSeed(s);

    cout << "Demonstrate simple voting by spatial actors (scalar capability)" << endl;
    cout << "and by economic actors (vector capability)" << endl;

    unsigned int sDim = 3;
    auto sp1 = new VctrPstn(KMatrix::uniform(rng, sDim, 1, 0.0, 1.0));
    cout << "Random spatial position, sp1:" << endl;
    sp1->printf(" %.3f ");
    cout << endl << flush;
    auto sp2 = new VctrPstn(KMatrix::uniform(rng, sDim, 1, 0.0, 1.0));
    cout << "Random spatial position, sp2:" << endl;
    sp2->printf(" %.3f ");
    cout << endl << flush;
    auto alice = new SMPActor("Alice", "first cryptographer");
    alice->randomize(rng, sDim);
    auto md0 = new SMPModel(rng);
    auto st0 = new SMPState(md0);
    st0->nra = KMatrix(1, 1); //
    auto iPos = new VctrPstn((2 * (*sp1) + (*sp2)) / 3);
    md0->addActor(alice);
    st0->addPstn(iPos);
    cout << "Alice's position is (2*sp1 + sp2)/3:" << endl;
    iPos->printf(" %.3f ");
    cout << endl << flush;
    printf("Alice's scalar capability: %.3f \n", alice->sCap);
    cout << "Alice's voting rule (overall): " << vrName(alice->vr) << endl;
    printf("Alice's risk attitude: %.3f \n", st0->nra(0, 0));
    printf("Alice's total salience %.4f \n", sum(alice->vSal));
    printf("Alice's vector salience: \n");
    alice->vSal.printf(" %.3f ");


    double va = alice->vote(sp1, sp2, st0);
    printf("A's vote on [sp1:sp2] is %+.3f \n", va);
    printf("Her vote should always be positive \n");
    cout << flush;
    assert(va > 0); // by construction
    cout << endl << flush;

    delete sp1;
    delete sp2;
    delete alice;
    delete st0;
    delete md0;


    return;
}


void demoEUSpatial(unsigned int numA, unsigned int sDim, uint64_t s, PRNG* rng) {
    assert(0 < sDim);
    assert(2 < numA);

    printf("Using PRNG seed: %020lu \n", s);
    rng->setSeed(s);

    cout << "EU State for SMP actors with scalar capabilities" << endl;
    printf("Number of actors; %i \n", numA);
    printf("Number of SMP dimensions %i \n", sDim);

    // note that because all actors use the same scale for capability, utility, etc,
    // their 'votes' are on the same scale and influence can be added up meaningfully

    const unsigned int maxIter = 5000;
    double qf = 100.0;
    auto md0 = new SMPModel(rng);
    md0->stop = [maxIter] (unsigned int iter, const State * s) {
        return (maxIter <= iter);
    };
    md0->stop = [maxIter, qf] (unsigned int iter, const State * s) {
        bool tooLong = (maxIter <= iter);
        bool quiet = false;
        if (1 < iter) {
            auto sf = [](unsigned int i1, unsigned int i2, double d12) {
                printf("sDist [%2i,%2i] = %.2E   ", i1, i2, d12);
                return;
            };

            auto s0 = ((const SMPState*) (s->model->history[0]));
            auto s1 = ((const SMPState*) (s->model->history[1]));
            auto d01 = SMPModel::stateDist(s0, s1);
            sf(0, 1, d01);

            auto sx = ((const SMPState*) (s->model->history[iter-0]));
            auto sy = ((const SMPState*) (s->model->history[iter-1]));
            auto dxy = SMPModel::stateDist(sx, sy);
            sf( iter-0, iter-1, dxy);

            quiet = (dxy < d01/qf);
            if (quiet)
                printf("Quiet \n");
            else
                printf("Not Quiet \n");

            cout << endl<<flush;
        }
        return tooLong || quiet;

    };
    md0->numDim = sDim;

    auto st0 = new SMPState(md0);
    md0->addState(st0); // now state 0 of the history

    st0->step = [st0]() {
        return st0->stepBCN();
    };

    for (unsigned int i = 0; i < numA; i++) {
        //string ni = "SActor-";
        //ni.append(std::to_string(i));
        unsigned int nbSize = 15;
        char * nameBuff = new char[nbSize];
        for (unsigned int j = 0; j < nbSize; j++) {
            nameBuff[j] = (char)0;
        }
        sprintf(nameBuff, "SActor-%02i", i);
        auto ni = string(nameBuff);
        delete nameBuff;
        nameBuff = nullptr;
        string di = "Random spatial actor";


        auto ai = new SMPActor(ni, di);
        ai->randomize(rng, sDim);
        auto iPos = new VctrPstn(KMatrix::uniform(rng, sDim, 1, 0.0, 1.0)); // SMP is always on [0,1] scale
        md0->addActor(ai);
        st0->addPstn(iPos);
    }
    st0->nra = KMatrix::uniform(rng, numA, 1, -0.5, 1.0);

    for (unsigned int i = 0; i < numA; i++) {
        auto ai = ((SMPActor*)(md0->actrs[i]));
        auto ri = st0->nra(i, 0);
        printf("%2i: %s , %s \n", i, ai->name.c_str(), ai->desc.c_str());
        cout << "voting rule: " << vrName(ai->vr) << endl;
        cout << "Pos vector: ";
        VctrPstn * pi = ((VctrPstn*)(st0->pstns[i]));
        trans(*pi).printf(" %+7.4f ");
        cout << "Sal vector: ";
        trans(ai->vSal).printf(" %+7.4f ");
        printf("Capability: %.3f \n", ai->sCap);
        printf("Risk attitude: %+.4f \n", ri);
        cout << endl;
    }

    // with SMP actors, we can always read their ideal position.
    // with strategic voting, they might want to advocate positions
    // separate from their ideal, but this simple demo skips that.
    auto uFn1 = [st0](unsigned int i, unsigned int j) {
        auto ai = ((SMPActor*)(st0->model->actrs[i]));
        auto pj = ((VctrPstn*)(st0->pstns[j])); // aj->iPos;
        double uij = ai->posUtil(pj, st0);
        return uij;
    };

    auto u = KMatrix::map(uFn1, numA, numA);
    cout << "Raw actor-pos util matrix" << endl;
    u.printf(" %.4f ");
    cout << endl << flush;

    auto w = st0->actrCaps(); //  KMatrix::map(wFn, 1, numA);

    // arbitrary but illustrates that we can do an election with arbitrary
    // voting rules - not necessarily the same as the actors would do.
    auto vr = VotingRule::Binary;
    cout << "Using voting rule " << vrName(vr) << endl;

    auto vpm = Model::VPModel::Linear;

    KMatrix p = Model::scalarPCE(numA, numA, w, u, vr, vpm, ReportingLevel::Medium);

    cout << "Expected utility to actors: " << endl;
    (u*p).printf(" %.3f ");
    cout << endl << flush;

    cout << "Net support for positions: " << endl;
    (w*u).printf(" %.3f ");
    cout << endl << flush;

    auto aCorr = [](const KMatrix & x, const KMatrix & y) {
        using KBase::lCorr;
        using KBase::mean;
        return  lCorr(x - mean(x), y - mean(y));
    };

    // for nearly flat distributions, and nearly flat net support,
    // one can sometimes see negative affine-correlations because of
    // random variations in 3rd or 4th decimal places.
    printf("L-corr of prob and net support: %+.4f \n", KBase::lCorr((w*u), trans(p)));
    printf("A-corr of prob and net support: %+.4f \n", aCorr((w*u), trans(p)));

    // no longer need external reference to the state
    st0 = nullptr;

    cout << "Starting model run" << endl << flush;
    md0->run();

    cout << "Completed model run" << endl << endl;

    cout << "History of actor positions over time" << endl;
    md0->showVPHistory();

    cout << endl;
    cout << "Delete model (actors, states, positions, etc.)" << endl << flush;
    delete md0;
    md0 = nullptr;

    return;
}

void readEUSpatial(uint64_t seed, string inputCSV, PRNG* rng) {
    auto md0 = SMPModel::readCSV(inputCSV, rng);

    const unsigned int maxIter = 5;
    md0->stop = [maxIter](unsigned int iter, const State * s) {
        return (maxIter <= iter);
    };

    cout << "Starting model run" << endl << flush;
    md0->run();

    cout << "Completed model run" << endl << endl;

    cout << "History of actor positions over time" << endl;
    md0->showVPHistory();

    delete md0;
    return;
}

} // end of namespace


int main(int ac, char **av) {
    using std::cout;
    using std::endl;
    using std::string;

    auto sTime = KBase::displayProgramStart();
    uint64_t dSeed = 0xD67CC16FE69C2868;  // arbitrary
    uint64_t seed = dSeed;
    bool run = true;
    bool euSmpP = false;
    bool csvP = false;
    string inputCSV = "";

    cout << "smpApp version " << DemoSMP::appVersion << endl << endl;

    auto showHelp = [dSeed]() {
        printf("\n");
        printf("Usage: specify one or more of these options\n");
        printf("--help            print this message\n");
        printf("--euSMP           exp. util. of spatial model of politics\n");
        printf("--csv <f>         read a scenario from CSV\n");
        printf("--seed <n>        set a 64bit seed\n");
        printf("                  0 means truly random\n");
        printf("                  default: %020lu \n", dSeed);
    };

    // tmp args

    if (ac > 1) {
        for (int i = 1; i < ac; i++) {
            if (strcmp(av[i], "--seed") == 0) {
                i++;
                seed = std::stoull(av[i]);
            }
            else if (strcmp(av[i], "--csv") == 0) {
                csvP = true;
                i++;
                inputCSV = av[i];
            }
            else if (strcmp(av[i], "--euSMP") == 0) {
                euSmpP = true;
            }
            else if (strcmp(av[i], "--help") == 0) {
                run = false;
            }
            else {
                run = false;
                printf("Unrecognized argument %s\n", av[i]);
            }
        }
    }

    if (!run) {
        showHelp();
        return 0;
    }

    PRNG * rng = new PRNG();
    seed = rng->setSeed(seed); // 0 == get a random number
    printf("Using PRNG seed:  %020llu \n", seed);
    printf("Same seed in hex:   0x%016llX \n", seed);

    // note that we reset the seed every time, so that in case something
    // goes wrong, we need not scroll back too far to find the
    // seed required to reproduce the bug.
    if (euSmpP) {
        cout << "-----------------------------------" << endl;
        DemoSMP::demoEUSpatial(7, 3, seed, rng);
    }
    if (csvP) {
        cout << "-----------------------------------" << endl;
        DemoSMP::readEUSpatial(seed, inputCSV, rng);
    }
    cout << "-----------------------------------" << endl;


    delete rng;
    KBase::displayProgramEnd(sTime);
    return 0;
}

// --------------------------------------------
// Copyright KAPSARC. Open source MIT License.
// --------------------------------------------