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

#include "smp.h"
#include "csv_parser.hpp"


namespace SMPLib {
using std::cout;
using std::endl;
using std::flush;
using std::function;
using std::get;
using std::string;

using KBase::PRNG;
using KBase::KMatrix;
using KBase::KException;
using KBase::Actor;
using KBase::Model;
using KBase::Position;
using KBase::State;
using KBase::VotingRule;
using KBase::ReportingLevel;


auto newChars = [](unsigned int len) {
    auto rslt = new char[len];
    for (unsigned int i = 0; i < len; i++) {
        rslt[i] = ((char)0);
    }
    return rslt;
};


BargainSMP::BargainSMP(const SMPActor* ai, const SMPActor* ar, const VctrPstn & pi, const VctrPstn & pr) {
    assert(nullptr != ai);
    assert(nullptr != ar);
    actInit = ai;
    actRcvr = ar;
    posInit = pi;
    posRcvr = pr;
}

BargainSMP::~BargainSMP() {
    actInit = nullptr;
    actRcvr = nullptr;
    posInit = KMatrix(0, 0);
    posRcvr = KMatrix(0, 0);
}

SMPActor::SMPActor(string n, string d) : Actor(n, d) {
    vr = VotingRule::Proportional; // just a default
}

SMPActor::~SMPActor() {
}

double SMPActor::vote(unsigned int, unsigned int, const State*) const {
    assert(false);  // TDO: finish this
    return 0;
}


double SMPActor::vote(const Position * ap1, const Position * ap2, const SMPState* ast) const {
    double u1 = posUtil(ap1, ast);
    double u2 = posUtil(ap2, ast);
    double v = Model::vote(vr, sCap, u1, u2);
    return v;
}


double SMPActor::posUtil(const Position * ap1, const SMPState* as) const {
    assert(nullptr != as);
    int ai = as->model->actrNdx(this);
    double ri = as->nra(ai, 0);
    assert(0 <= ai);
    const VctrPstn* p0 = ((const VctrPstn*)(as->pstns[ai]));
    assert(nullptr != p0);
    auto p1 = ((const VctrPstn*)ap1);
    assert(nullptr != p1);
    double u1 = SMPModel::bvUtil((*p0) - (*p1), vSal, ri);
    return u1;
}


void SMPActor::randomize(PRNG* rng, unsigned int numD) {
    sCap = rng->uniform(10.0, 200.0);

    // assign an overall salience, and then by-component saliences
    double s = rng->uniform(0.75, 0.99);
    vSal = KMatrix::uniform(rng, numD, 1, 0.1, 1.0);
    vSal = (s * vSal) / sum(vSal);
    assert(fabs(s - sum(vSal)) < 1E-4);

    // I could randomly assign different voting rules
    // to different actors, but that would just be too cute.
    vr = VotingRule::Proportional;

    return;
}



void SMPActor::interpBrgnSnPm(unsigned int n, unsigned int m,
                              double tik, double sik, double prbI,
                              double tjk, double sjk, double prbJ,
                              double & bik, double & bjk) {
    assert((1 == n) || (2 == n));
    assert((1 == m) || (2 == m));

    double wsi = pow(sik, n);
    double wpi = pow(prbI, m);
    double wik = wsi * wpi;

    double wsj = pow(sjk, n);
    double wpj = pow(prbJ, m);
    double wjk = wsj * wpj;

    // imagine that either neither actor cares, or neither actor can coerce the other,
    // so that wik = 0 = wjk. We need to avoid 0/0 error, and have bi=ti and bj=tj.
    const double minW = 1e-6;
    bik = ((wik + minW)*tik + wjk*tjk) / (wik + minW + wjk);
    bjk = (wik*tik + (minW + wjk)*tjk) / (wik + minW + wjk);
    return;
}


void SMPActor::interpBrgnS2PMax(double tik, double sik, double prbI,
                                double tjk, double sjk, double prbJ,
                                double & bik, double & bjk) {
    double di = (prbJ > prbI) ? (prbJ - prbI) : 0;  // max(0, prbJ - prbI);
    double dj = (prbI > prbJ) ? (prbI - prbJ) : 0;  // max(0, prbI - prbJ);
    double sik2 = sik * sik;
    double sjk2 = sjk * sjk;

    const double minW = 1e-6;
    double dik = (di * sjk2) / ((di * sjk2) + minW + ((1 - di) * sik2));
    double djk = (dj * sik2) / ((dj * sik2) + minW + ((1 - dj) * sjk2));


    bik = tik + dik * (tjk - tik);
    bjk = tjk + djk * (tik - tjk);
    return;
}


BargainSMP* SMPActor::interpolateBrgn(const SMPActor* ai, const SMPActor* aj,
                                      const VctrPstn* posI, const VctrPstn * posJ,
                                      double prbI, double prbJ, InterVecBrgn ivb)  {
    assert((1 == posI->numC()) && (1 == posJ->numC()));
    unsigned int numD = posI->numR();
    assert(numD == posJ->numR());
    auto brgnI = VctrPstn(numD, 1);
    auto brgnJ = VctrPstn(numD, 1);

    for (unsigned int k = 0; k < numD; k++) {
        double tik = (*posI)(k, 0);
        double sik = ai->vSal(k, 0);

        double tjk = (*posJ)(k, 0);
        double sjk = aj->vSal(k, 0);
        double & bik = tik;
        double & bjk = tjk;
        switch (ivb) {
        case InterVecBrgn::S1P1:
            interpBrgnSnPm(1, 1, tik, sik, prbI, tjk, sjk, prbJ, bik, bjk);
            break;
        case InterVecBrgn::S2P2:
            interpBrgnSnPm(2, 2, tik, sik, prbI, tjk, sjk, prbJ, bik, bjk);
            break;
        case InterVecBrgn::S2PMax:
            interpBrgnS2PMax(tik, sik, prbI, tjk, sjk, prbJ, bik, bjk);
            break;
        default:
            throw KException("interpolateBrgn: unrecognized InterVecBrgn value");
            break;
        }
        brgnI(k, 0) = bik;
        brgnJ(k, 0) = bjk;
    }

    auto brgn = new BargainSMP(ai, aj, brgnI, brgnJ);
    return brgn;
}


KMatrix SMPState::bigRfromProb(const KMatrix& p, BigRRange rr)   {
    double pMin = 1.0;
    double pMax = 0.0;
    for (double pi : p) {
        assert(0.0 <= pi);
        assert(pi <= 1.0);
        pMin = (pi < pMin) ? pi : pMin;
        pMax = (pi > pMax) ? pi : pMax;
    }

    const double pTol = 1E-8;
    assert(fabs(1 - KBase::sum(p)) < pTol);

    function<double(unsigned int, unsigned int)> rfn = nullptr;
    switch (rr) {
    case BigRRange::Min:
        rfn = [pMin, pMax, p](unsigned int i, unsigned int j) {
            return (p(i, j) - pMin) / (pMax - pMin);
        };
        break;
    case BigRRange::Mid:
        rfn = [pMin, pMax, p](unsigned int i, unsigned int j) {
            return (3 * p(i, j) - (pMax + 2 * pMin)) / (2 * (pMax - pMin));
        };
        break;
    case BigRRange::Max:
        rfn = [pMin, pMax, p](unsigned int i, unsigned int j) {
            return (2 * p(i, j) - (pMax + pMin)) / (pMax - pMin);
        };
        break;
    }
    auto rMat = KMatrix::map(rfn, p.numR(), p.numC());
    return rMat;
}


SMPState::SMPState(Model * m) : State(m) {
    nra = KMatrix();
}


SMPState::~SMPState() {
    nra = KMatrix();
}



void SMPState::setDiff() {
    auto dfn = [this](unsigned int i, unsigned int j) {
        auto ai = ((const SMPActor*)(model->actrs[i]));
        KMatrix si = ai->vSal;
        auto pi = ((const VctrPstn*)(pstns[i]));
        auto pj = ((const VctrPstn*)(pstns[j]));
        const double d = SMPModel::bvDiff((*pi) - (*pj), si);
        return d;
    };

    const unsigned int na = model->numAct;
    diff = KMatrix::map(dfn, na, na);
    return;
}


double SMPState::estNRA(unsigned int h, unsigned int i, SMPState::BigRAdjust ra) const {
    double rh = nra(h, 0);
    double ri = nra(i, 0);
    double rhi = 0.0;
    switch (ra) {
    case SMPState::BigRAdjust::Full:
        rhi = ri;
        break;
    case SMPState::BigRAdjust::Half:
        rhi = (rh + ri) / 2;
        break;
    case SMPState::BigRAdjust::None:
        rhi = rh;
        break;
    }
    return rhi;
}

KMatrix SMPState::actrCaps() const {
    auto wFn = [this](unsigned int i, unsigned int j) {
        auto aj = ((SMPActor*)(model->actrs[j]));
        return aj->sCap;
    };

    auto w = KMatrix::map(wFn, 1, model->numAct);
    return w;
}

void SMPState::setAUtil(ReportingLevel rl) {
    // you can change these parameters
    auto vr = VotingRule::Proportional;
    auto ra = SMPState::BigRAdjust::Half;
    auto rr = BigRRange::Mid; // use [-0.5, +1.0] scale
    auto vpm = Model::VPModel::Linear;

    const unsigned int na = model->numAct;
    auto w_j = actrCaps();
    setDiff();
    nra = KMatrix(na, 1); // zero-filled, i.e. risk neutral
    auto uFn1 = [this](unsigned int i, unsigned int j) {
        return  SMPModel::bsUtil(diff(i, j), nra(i, 0));
    };

    auto rnUtil_ij = KMatrix::map(uFn1, na, na);

    if (ReportingLevel::Silent < rl) {
        cout << "Raw actor-pos value matrix (risk neutral)" << endl;
        rnUtil_ij.printf(" %+.3f ");
        cout << endl << flush;
    }

    auto pv_ij = Model::vProb(vr, vpm, w_j, rnUtil_ij);
    auto p_i = Model::probCE(pv_ij);
    nra = bigRfromProb(p_i, rr);


    if (ReportingLevel::Silent < rl) {
        cout << "Inferred risk attitudes: " << endl;
        nra.printf(" %+.3f ");
        cout << endl << flush;
    }

    auto raUtil_ij = KMatrix::map(uFn1, na, na);

    if (ReportingLevel::Silent < rl) {
        cout << "Risk-aware actor-pos utility matrix (objective):" << endl;
        raUtil_ij.printf(" %+.4f ");
        cout << endl;
        cout << "RMS change in value vs utility: " << norm(rnUtil_ij - raUtil_ij) / na << endl << flush;
    }

    const double duTol = 1E-6;
    assert(duTol < norm(rnUtil_ij - raUtil_ij)); // I've never seen it below 0.07


    if (ReportingLevel::Silent < rl) {
        switch (ra) {
        case SMPState::BigRAdjust::Full:
            cout << "Using Full adjustment of ra, r^h_i = ri" << endl;
            break;
        case SMPState::BigRAdjust::Half:
            cout << "Using Half adjustment of ra, r^h_i = (rh + ri)/2" << endl;
            break;
        case SMPState::BigRAdjust::None:
            cout << "Using None adjustment of ra, r^h_i = rh " << endl;
            break;
        default:
            cout << "Unrecognized SMPState::BigRAdjust" << endl;
            assert(false);
            break;
        }
    }

    aUtil = vector<KMatrix>();
    for (unsigned int h = 0; h < na; h++) {
        auto u_h_ij = KMatrix(na, na);
        for (unsigned int i = 0; i < na; i++) {
            double rhi = estNRA(h, i, ra);
            for (unsigned int j = 0; j < na; j++) {
                double dij = diff(i, j);
                u_h_ij(i, j) = SMPModel::bsUtil(dij, rhi);
            }
        }
        aUtil.push_back(u_h_ij);


        if (ReportingLevel::Silent < rl) {
            cout << "Estimate by " << h << " of risk-aware utility matrix:" << endl;
            u_h_ij.printf(" %+.4f ");
            cout << endl;

            cout << "RMS change in util^h vs utility: " << norm(u_h_ij - raUtil_ij) / na << endl;
            cout << endl;
        }

        assert(duTol < norm(u_h_ij - raUtil_ij)); // I've never seen it below 0.03
    }
    return;
}


void SMPState::showBargains(const vector < vector < BargainSMP* > > & brgns) const {
    for (unsigned int i = 0; i < brgns.size(); i++) {
        printf("Bargains involving actor %i: ", i);
        for (unsigned int j = 0; j < brgns[i].size(); j++) {
            BargainSMP* bij = brgns[i][j];
            if (nullptr != bij) {
                int a1 = model->actrNdx(bij->actInit);
                int a2 = model->actrNdx(bij->actRcvr);
                printf(" [%i:%i] ", a1, a2);
            }
            else {
                printf(" SQ ");
            }
        }
        cout << endl << flush;
    }
    return;
}


void SMPState::addPstn(Position* ap) {
    auto sp = (VctrPstn*)ap;
    auto sm = (SMPModel*)model;
    assert(1 == sp->numC());
    assert(sm->numDim == sp->numR());

    State::addPstn(ap);
    return;
}


bool SMPState::equivNdx(unsigned int i, unsigned int j) const {
    /// Compare two actual positions in the current state
    auto vpi = ((const VctrPstn *) (pstns[i]));
    auto vpj = ((const VctrPstn *) (pstns[j]));
    assert (vpi != nullptr);
    assert (vpj != nullptr);
    double diff = norm( (*vpi) - (*vpj));
    auto sm = ((const SMPModel *) model);
    bool rslt = (diff < sm->posTol);
    return rslt;
}


// set the diff matrix, do probCE for risk neutral,
// estimate Ri, and set all the aUtil[h] matrices
SMPState* SMPState::stepBCN() {
    setAUtil(ReportingLevel::Low);
    auto s2 = doBCN();
    s2->step = [s2]() {
        return s2->stepBCN();
    };
    return s2;
}


SMPState* SMPState::doBCN() const {
    auto brgns = vector< vector < BargainSMP* > >();
    const unsigned int na = model->numAct;
    brgns.resize(na);
    for (unsigned int i = 0; i < na; i++) {
        brgns[i] = vector<BargainSMP*>();
        brgns[i].push_back(nullptr); // null bargain is SQ
    }

    auto ivb = SMPActor::InterVecBrgn::S2P2;
    // For each actor, identify good targets, and propose bargains to them.
    // (This loop would be an excellent place for high-level parallelism)
    for (unsigned int i = 0; i < na; i++) {
        auto chlgI = bestChallenge(i);
        int bestJ = get<0>(chlgI);
        double piJ = get<1>(chlgI);
        double bestEU = get<2>(chlgI);
        if (0 < bestEU) {
            assert(0 <= bestJ);

            printf("Actor %i has most advantageous target %i worth %.3f\n", i, bestJ, bestEU);

            auto ai = ((const SMPActor*)(model->actrs[i]));
            auto aj = ((const SMPActor*)(model->actrs[bestJ]));
            auto posI = ((const VctrPstn*)pstns[i]);
            auto posJ = ((const VctrPstn*)pstns[bestJ]);
            BargainSMP* brgnIJ = SMPActor::interpolateBrgn(ai, aj, posI, posJ, piJ, 1 - piJ, ivb);
            auto nai = model->actrNdx(brgnIJ->actInit);
            auto naj = model->actrNdx(brgnIJ->actRcvr);

            brgns[i].push_back(brgnIJ); // initiator's copy, delete only it later
            brgns[bestJ].push_back(brgnIJ); // receiver's copy, just null it out later

            printf(" %2i proposes %2i adopt: ", nai, nai);
            KBase::trans(brgnIJ->posInit).printf(" %.3f ");
            printf(" %2i proposes %2i adopt: ", nai, naj);
            KBase::trans(brgnIJ->posRcvr).printf(" %.3f ");
        }
        else {
            printf("Actor %i has no advantageous targets \n", i);
        }
    }


    cout << endl << "Bargains to be resolved" << endl << flush;
    showBargains(brgns);

    auto w = actrCaps();
    cout << "w:" << endl;
    w.printf(" %6.2f ");

    // of course, you  can change these two parameters
    auto vr = VotingRule::Proportional;
    auto vpm = Model::VPModel::Linear;

    auto ndxMaxProb = [](const KMatrix & cv) {

        const double pTol = 1E-8;
        assert(fabs(KBase::sum(cv) - 1.0) < pTol);

        assert(0 < cv.numR());
        assert(1 == cv.numC());

        auto ndxIJ = ndxMaxAbs(cv);
        unsigned int iMax = get<0>(ndxIJ);
        return iMax;
    };

    // what is the utility to actor nai of the state resulting after
    // the nbj-th bargain of the k-th actor is implemented?
    auto brgnUtil = [this, brgns](unsigned int nk, unsigned int nai, unsigned int nbj) {
        const unsigned int na = model->numAct;
        BargainSMP * b = brgns[nk][nbj];
        double uAvrg = 0.0;

        if (nullptr == b) { // SQ bargain
            uAvrg = 0.0;
            for (unsigned int n = 0; n < na; n++) {
                // nai's estimate of the utility to nai of position n, i.e. the true value
                uAvrg = uAvrg + aUtil[nai](nai, n);
            }
        }

        if (nullptr != b) { // all positions unchanged, except Init and Rcvr
            uAvrg = 0.0;
            auto ndxInit = model->actrNdx(b->actInit);
            assert((0 <= ndxInit) && (ndxInit < na)); // must find it
            double uPosInit = ((SMPActor*)(model->actrs[nai]))->posUtil(&(b->posInit), this);
            uAvrg = uAvrg + uPosInit;

            auto ndxRcvr = model->actrNdx(b->actRcvr);
            assert((0 <= ndxRcvr) && (ndxRcvr < na)); // must find it
            double uPosRcvr = ((SMPActor*)(model->actrs[nai]))->posUtil(&(b->posRcvr), this);
            uAvrg = uAvrg + uPosRcvr;

            for (unsigned int n = 0; n < na; n++) {
                if ((ndxInit != n) && (ndxRcvr != n)) {
                    // again, nai's estimate of the utility to nai of position n, i.e. the true value
                    uAvrg = uAvrg + aUtil[nai](nai, n);
                }
            }
        }

        uAvrg = uAvrg / na;

        assert(0.0 < uAvrg); // none negative, at least own is positive
        assert(uAvrg <= 1.0); // can not all be over 1.0
        return uAvrg;
    };
    // end of λ-fn


    // TODO: finish this
    // For each actor, assess what bargains result from CDMP, and put it into s2

    // The key is to build the usual matrix of U_ai (Brgn_m) for all bargains in brgns[k],
    // making sure to divide the sum of the utilities of positions by 1/N
    // so 0 <= Util(state after Brgn_m) <= 1, then do the standard scalarPCE for bargains involving k.


    SMPState* s2 = new SMPState(model);
    // (This loop would be a good place for high-level parallelism)
    for (unsigned int k = 0; k < na; k++) {
        unsigned int nb = brgns[k].size();
        auto buk = [brgnUtil, k](unsigned int nai, unsigned int nbj) {
            return brgnUtil(k, nai, nbj);
        };
        auto u_im = KMatrix::map(buk, na, nb);

        cout << "u_im: " << endl;
        u_im.printf(" %.5f ");

        cout << "Doing probCE for the " << nb << " bargains of actor " << k << " ... " << flush;
        auto p = Model::scalarPCE(na, nb, w, u_im, vr, vpm, ReportingLevel::Medium);
        assert(nb == p.numR());
        assert(1 == p.numC());
        cout << "done" << endl << flush;
        unsigned int mMax = ndxMaxProb(p); // indexing actors by i, bargains by m
        cout << "Chosen bargain: " << mMax << endl;



        // TODO: create a fresh position for k, from the selected bargain mMax.
        VctrPstn * pk = nullptr;
        auto bkm = brgns[k][mMax];
        if (nullptr == bkm) {
            auto oldPK = (VctrPstn *)pstns[k];
            pk = new VctrPstn(*oldPK);
        }
        else {
            const unsigned int ndxInit = model->actrNdx(bkm->actInit);
            const unsigned int ndxRcvr = model->actrNdx(bkm->actRcvr);
            if (ndxInit == k) {
                pk = new VctrPstn(bkm->posInit);
            }
            else if (ndxRcvr == k) {
                pk = new VctrPstn(bkm->posRcvr);
            }
            else {
                cout << "SMPState::doBCN: unrecognized actor in bargain" << endl;
                assert(false);
            }
        }
        assert(nullptr != pk);

        assert(k == s2->pstns.size());
        s2->pstns.push_back(pk);

        cout << endl << flush;
    }


    // Some bargains are nullptr, and there are two copies of every non-nullptr randomly
    // arranged. If we delete them as we find them, then the second occurance will be corrupted,
    // so the code crashes when it tries to access the memory to see if it matches something
    // already deleted. Hence, we scan them all, building a list of unique bargains,
    // then delete those in order.
    auto uBrgns = vector<BargainSMP*>();

    for (unsigned int i = 0; i < brgns.size(); i++) {
        auto ai = ((const SMPActor*)(model->actrs[i]));
        for (unsigned int j = 0; j < brgns[i].size(); j++) {
            BargainSMP* bij = brgns[i][j];
            if (nullptr != bij) {
                if (ai == bij->actInit) {
                    uBrgns.push_back(bij); // this is the initiator's copy, so save it for deletion
                }
            }
            brgns[i][j] = nullptr; // either way, null it out.
        }
    }

    for (auto b : uBrgns) {
        int aI = model->actrNdx(b->actInit);
        int aR = model->actrNdx(b->actRcvr);
        //printf("Delete bargain [%2i:%2i] \n", aI, aR);
        delete b;
    }

    return s2;
}


// h's estimate of the victory probability and expected change in utility for k from i challenging j,
// compared to status quo.
// Note that the  aUtil vector of KMatrix must be set before starting this.
// TODO: offer a choice the different ways of estimating value-of-a-state: even sum or expected value.
// TODO: we may need to separate euConflict from this at some point
tuple<double, double> SMPState::probEduChlg(unsigned int h, unsigned int k, unsigned int i, unsigned int j) const {

    // you could make other choices for these two sub-models
    auto vr = VotingRule::Proportional;
    auto tpc = KBase::ThirdPartyCommit::Semi;

    double uii = aUtil[h](i, i);
    double uij = aUtil[h](i, j);
    double uji = aUtil[h](j, i);
    double ujj = aUtil[h](j, j);

    // h's estimate of utility to k of status-quo positions of i and j
    double euSQ = aUtil[h](k, i) + aUtil[h](k, j);
    assert((0.0 <= euSQ) && (euSQ <= 2.0));

    // h's estimate of utility to k of i defeating j, so j adopts i's position
    double uhkij = aUtil[h](k, i) + aUtil[h](k, i);
    assert((0.0 <= uhkij) && (uhkij <= 2.0));

    // h's estimate of utility to k of j defeating i, so i adopts j's position
    double uhkji = aUtil[h](k, j) + aUtil[h](k, j);
    assert((0.0 <= uhkji) && (uhkji <= 2.0));

    auto ai = ((const SMPActor*)(model->actrs[i]));
    double si = KBase::sum(ai->vSal);
    double ci = ai->sCap;
    auto aj = ((const SMPActor*)(model->actrs[j]));
    double sj = KBase::sum(aj->vSal);
    assert((0 < sj) && (sj <= 1));
    double cj = aj->sCap;
    double minCltn = 1E-10;

    // h's estimate of i's unilateral influence contribution to (i:j), hence positive
    double contrib_i_ij = Model::vote(vr, si*ci, uii, uij);
    assert(0 <= contrib_i_ij);
    double chij = minCltn + contrib_i_ij; // strength of complete coalition supporting i over j
    assert (0.0 < chij);

    // h's estimate of j's unilateral influence contribution to (i:j), hence negative
    double contrib_j_ij = Model::vote(vr, sj*cj, uji, ujj);
    assert(contrib_j_ij <= 0);
    double chji = minCltn - contrib_j_ij; // strength of complete coalition supporting j over i
    assert (0.0 < chji);

    const unsigned int na = model->numAct;

    // we assess the overall coalition strengths by adding up the contribution of
    // individual actors (including i and j, above). We assess the contribution of third
    // parties by looking at little coalitions in the hypothetical (in:j) or (i:nj) contests.
    for (unsigned int n = 0; n < na; n++) {
        if ((n != i) && (n != j)) { // already got their influence-contributions
            auto an = ((const SMPActor*)(model->actrs[n]));
            double cn = an->sCap;
            double sn = KBase::sum(an->vSal);
            double uni = aUtil[h](n, i);
            double unj = aUtil[h](n, j);
            double unn = aUtil[h](n, n);

            double pin = Actor::vProbLittle(vr, sn*cn, uni, unj, chij, chji);
            assert(0.0 <= pin);
            assert(pin <= 1.0);
            double pjn = 1.0 - pin;

            double vnij = Actor::thirdPartyVoteSU(sn*cn, vr, tpc, pin, pjn, uni, unj, unn);

            chij = (vnij > 0) ? (chij + vnij) : chij;
            assert(0 < chij);
            chji = (vnij < 0) ? (chji - vnij) : chji;
            assert(0 < chji);
        }
    }

    const double phij = chij / (chij + chji);
    const double phji = chji / (chij + chji);

    const double euCh = (1 - sj)*uhkij + sj*(phij*uhkij + phji*uhkji);
    const double euChlg = euCh - euSQ;
    // printf ("SMPState::probEduChlg(%2i, %2i, %2i, %i2) = %+6.4f - %+6.4f = %+6.4f\n", h, k, i, j, euCh, euSQ, euChlg);
    auto rslt = tuple<double, double>(phij, euChlg);
    return rslt;
}



tuple<int, double, double> SMPState::bestChallenge(unsigned int i) const {
    int bestJ = -1;
    double piJ = 0;
    double bestEU = 0;

    // for SMP, positive ej are typically in the 0.5 to 0.01 range, so I take 1/1000 of the minimum,
    const double minSig = 1e-5;

    for (unsigned int j = 0; j < model->numAct; j++) {
        if (j != i) {
            auto pej = probEduChlg(i, i, i, j);
            double pj = get<0>(pej); // i's estimate of the victory-Prob for i challengeing j
            double ej = get<1>(pej); // i's estimate of the change in utility to i of i challengeing j, compared to SQ
            if ((minSig < ej) && (bestEU < ej)) {
                bestJ = j;
                piJ = pj;
                bestEU = ej;
            }
        }
    }
    auto rslt = tuple<int, double, double>(bestJ, piJ, bestEU);
    return rslt;
}


tuple< KMatrix, vector<unsigned int>> SMPState::pDist(int persp) const {
    auto na = model->numAct;
    auto w = actrCaps();
    auto vr = VotingRule::Proportional;
    auto rl = ReportingLevel::Silent;
    auto vpm = Model::VPModel::Linear;
    auto uij = KMatrix(na, na); // full utility matrix, including duplicate columns

    if ((0 <= persp) && (persp < na)) {
        uij = aUtil[persp];
    }
    else if (-1 == persp) {
        for (unsigned int i = 0; i < na; i++) {
            for (unsigned int j = 0; j < na; j++) {
                auto ui = aUtil[i];
                uij(i, j) = aUtil[i](i, j);
            }
        }
    }
    else {
        cout << "SMPState::pDist: unrecognized perspective, " << persp << endl << flush;
        assert(false);
    }
    auto pd = Model::scalarPCE(na, na, w, uij, vr, vpm, rl);
    auto uNdx = vector<unsigned int>();
    for (unsigned int i = 0; i < na; i++) {
        uNdx.push_back(i);
    }


    auto uNdx2 = uniqueNdx();
    printf("Unique positions %i/%i ", uNdx2.size(), uNdx.size());
    cout << "[ ";
    for (auto i : uNdx2) {
        printf(" %i ", i);
    }
    cout << " ] " << endl << flush;
    auto uufn = [uij, uNdx2](unsigned int i, unsigned int j) {
        return uij(i, uNdx2[j]);
    };
    auto uUij = KMatrix::map(uufn, na, uNdx2.size());
    auto upd = Model::scalarPCE(na, uNdx2.size(), w, uUij, vr, vpm, rl);

    //assert (uNdx.size() == uNdx2.size());

    //return tuple< KMatrix, vector<unsigned int>> (pd, uNdx);
    return tuple< KMatrix, vector<unsigned int>> (upd, uNdx2);
//    return pd;
}


// -------------------------------------------------


SMPModel::SMPModel(PRNG * r) : Model(r) {
    numDim = 0;
    posTol = 0.001;
    dimName = vector<string>();

    // just a test to get linkages correct

    auto callBack = [] (void *NotUsed, int argc, char **argv, char **azColName) {
        for(int i=0; i<argc; i++) {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
        return ((int) 0);
    };

    smpDB = nullptr;
    sqlite3 * db; // I don't like passing 'this' into lambda-functions
    char* zErrMsg = nullptr;
    int  rc;
    string sql;

    auto sOpen = [&db] (unsigned int n) {
        int rc = sqlite3_open("test.db", &db);
        if( rc != SQLITE_OK ) {
            fprintf(stdout, "Can't open database: %s\n", sqlite3_errmsg(db));
            exit(0);
        } else {
            fprintf(stdout, "Successfully opened database #%i\n", n);
        }
        cout << endl << flush;
        return;
    };

    auto sExec = [&db, callBack, &zErrMsg] (string sql, string msg) {
        int rc = sqlite3_exec(db, sql.c_str(), callBack, nullptr, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stdout, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, msg.c_str());
        }
        return rc;
    };

    // Open database
    cout << endl << flush;
    sOpen(1);

    // Create & execute SQL statements
    for (unsigned int i=0; i<12; i++ ) {
        auto buff = newChars(50);
        sprintf(buff, "Created table %i successfully \n", i);
        sql = createTableSQL(i);
        rc = sExec(sql, buff);
	buff = nullptr;
        cout << flush;
    }
    cout << endl << flush;

    smpDB = db;
}

SMPModel::~SMPModel() {
    if (nullptr != smpDB) {
      cout << "SMPModel::~SMPModel Closing database" << endl << flush;
        sqlite3_close(smpDB);
        smpDB = nullptr;
    }
}


string SMPModel::createTableSQL(unsigned int tn) {
    string sql ="";
    switch (tn) {
    case 0: // position-utility table
        sql = "create table PosUtil ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Act_i	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 1: // pos-vote table
        sql = "create table PosVote ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Voter_k	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_i	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_j	INTEGER NOT NULL DEFAULT 0, "\
              "Vote	REAL"\
              ");";
        break;
	
    case 2: // pos-prob table. Note that there may be duplicates, unless we limit it to unique positions
        sql = "create table PosProb ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_i	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL"\
              ");";
        break;
	
    case 3: // pos-equiv table. E(i)= lowest j s.t. Pos(i) ~ Pos(j). if j < i, it is not unique.
        sql = "create table PosEquiv ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_i	INTEGER NOT NULL DEFAULT 0, "\
              "Eqv_j	INTEGER NOT NULL DEFAULT 0 "\
              ");";
        break;
	
    case 4:  
        sql = "create table UtilContest ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Aff_k	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 5:  
        sql = "create table UtilChlg ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Aff_k	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 6:  
        sql = "create table UtilVict ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Aff_k	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 7:  
        sql = "create table ProbVict ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL"\
              ");";
        break;
	
    case 8:  
        sql = "create table UtilSQ ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Aff_k	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 9:  // probability ik > j
        sql = "create table ProbTPVict ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "ThrdP_k	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL"\
              ");";
        break;
	
    case 10: // utility to k of ik>j
        sql = "create table UtilTPVict ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "ThrdP_k	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;
	
    case 11: // utility to k of i>jk
        sql = "create table UtilTPLoss ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "ThrdP_k	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;

    default:
        throw(KException("SMPModel::createTableSQL unrecognized table number"));
    }

    return sql;
}

void SMPModel::addDim(string dn) {
    dimName.push_back(dn);
    numDim = dimName.size();
    return;
}

double SMPModel::stateDist(const SMPState* s1 , const SMPState* s2 ) {
    unsigned int n = s1->pstns.size();
    assert (n == s2->pstns.size());
    double dSum = 0;
    for (unsigned int i=0; i<n; i++) {
        auto vp1i = ((const VctrPstn*) (s1->pstns[i]));
        auto vp2i = ((const VctrPstn*) (s2->pstns[i]));
        dSum = dSum + KBase::norm((*vp1i) - (*vp2i));
    }
    return dSum;
}


// 0 <= d <= 1 is the difference in normalized position
// -1 <= R <= +1 is normalized risk-aversion
double SMPModel::bsUtil(double d, double R) {
    double u = 0;
    assert(0 <= d);
    if (d <= 1) {
        u = (1 - d)*(1 + d*R);  //  (0 <= u) && (u <= 1)
    }
    else {
        // linearly interpolate with last util-slope at d=1,
        // This is "unphysical", but we have to deal with it anyway
        // because VHCSearch will vary components out of physical limits
        // for both scalar and vector cases.
        double uSlope = -(R + 1); // obviously, uSlope <= 0
        u = uSlope*(d - 1); // because u=0 at d=1, regardless of R, this u<0
    }
    return u;
}

double SMPModel::bvDiff(const  KMatrix & d, const  KMatrix & s) {
    assert(KBase::sameShape(d, s));
    double dsSqr = 0;
    double ssSqr = 0;
    for (unsigned int i = 0; i < d.numR(); i++) {
        for (unsigned int j = 0; j < d.numC(); j++) {
            const double dij = d(i, j);
            const double sij = s(i, j);
            assert(0 <= sij);
            const double ds = dij * sij;
            const double s = sij;
            dsSqr = dsSqr + (ds*ds);
            ssSqr = ssSqr + (s*s);
        }
    }
    assert(0 < ssSqr);
    double sd = sqrt(dsSqr / ssSqr);
    return sd;
};

double SMPModel::bvUtil(const  KMatrix & d, const  KMatrix & s, double R) {
    const double sd = bvDiff(d, s);
    const double u = bsUtil(sd, R);
    return u;
};

/// the probability of the position occupied by actor i
double SMPState::posProb(unsigned int i, vector<unsigned int> unq, const KMatrix & pdt) const {
    const unsigned int numA = model->numAct;
    unsigned int k = numA + 1; // impossibly high
    for (unsigned int j1=0; j1<unq.size(); j1++) { // scan unique positions
        unsigned int j2 = unq[j1]; // get ordinary index of the position
        if (equivNdx(i,j2)) {
            k = j1;
        }
    }
    assert (k < numA);
    assert (k < pdt.numR());
    double pr = pdt(k,0);
    return pr;
}

void SMPModel::showVPHistory() const {
    const string commaSep = " , ";

    // show positions over time
    for (unsigned int i = 0; i < numAct; i++) {
        for (unsigned int k = 0; k < numDim; k++) {
            cout << actrs[i]->name << commaSep;
            printf("Dim-%02i %s", k, commaSep.c_str());
            //cout << "Dim-"<< k << commaSep;
            for (unsigned int t = 0; t < history.size(); t++) {
                State* st = history[t];
                Position* pit = st->pstns[i];
                VctrPstn* vpit = (VctrPstn*)pit;
                assert(1 == vpit->numC());
                assert(numDim == vpit->numR());
                printf("%7.3f %s", 100 * (*vpit)(k, 0), commaSep.c_str());
            }
            cout << endl;
        }
    }
    cout << endl;
    // show probabilities over time.
    // Note that we have to set the aUtil matrices for the last one.
    auto prbHist = vector<KMatrix>();
    auto unqHist = vector< vector<unsigned int>>();
    for (unsigned int t = 0; t < history.size(); t++) {
        auto sst = (SMPState*)history[t];
        if (t == history.size() - 1) {
            sst->setAUtil(ReportingLevel::Silent);
        }
        auto pn = sst->pDist(-1);
        auto pdt = std::get<0>(pn); // TODO: check for equivalent positions
        auto unq = std::get<1>(pn);
        prbHist.push_back(pdt);
        unqHist.push_back(unq);
    }

    auto probIT = [this, prbHist, unqHist] (unsigned int i, unsigned int t) {
        auto pdt = prbHist[t];
        auto unq = unqHist[t];
        auto sst = ((SMPState*) (history[t]));
        double pr = sst->posProb(i, unq, pdt);
        return pr;
    };

    // TODO: displaying the probabilities of actors winning is a bit odd,
    // as we display the probability of their position winning. As multiple
    // actors often occupy the equivalent positions, this means the displayed probabilities
    // will often add up to more than 1.
    for (unsigned int i = 0; i < numAct; i++) {
        cout << actrs[i]->name << commaSep;
        printf("prob %s", commaSep.c_str());
        for (unsigned int t = 0; t < history.size(); t++) {
            printf("%.4f %s", probIT(i,t), commaSep.c_str()); //  prbHist[t](i, 0),
        }
        cout << endl << flush;
    }
    return;
}


SMPModel * SMPModel::readCSV(string fName, PRNG * rng) {
    using KBase::KException;

    const unsigned int minNumActor = 3;
    const unsigned int maxNumActor = 100; // It's just a demo
    char * errBuff = newChars(100); // as sprintf requires

    csv_parser csv(fName);

    // Get values according to row number and column number.
    // Remember it starts from (1,1) and not (0,0)
    string scenName = csv.get_value(1, 1);
    cout << "Scenario name: |" << scenName << "|" << endl;
    cout << flush;
    string numActorString = csv.get_value(1, 3);
    unsigned int numActor = atoi(numActorString.c_str());
    string numDimString = csv.get_value(1, 4);
    int numDim = atoi(numDimString.c_str());
    printf("Number of actors: %i \n", numActor);
    printf("Number of dimensions: %i \n", numDim);
    cout << endl << flush;

    if (numDim < 1) { // lower limit
        throw(KBase::KException("SMPModel::readCSV: Invalid number of dimensions"));
    }
    assert(0 < numDim);
    if ((numActor < minNumActor) || (maxNumActor < numActor)) { // avoid impossibly low or ridiculously large
        throw(KBase::KException("SMPModel::readCSV: Invalid number of actors"));
    }
    assert(minNumActor <= numActor);
    assert(numActor <= maxNumActor);

    // Read actor data
    auto actorNames = vector<string>();
    auto actorDescs = vector<string>();
    auto cap = KMatrix(numActor, 1);
    auto nra = KMatrix(numActor, 1);
    for (unsigned int i = 0; i < numActor; i++) {
        // get short names
        string nis = csv.get_value(3 + i, 1);
        assert(0 < nis.length());
        actorNames.push_back(nis);
        printf("Actor %3i name: %s \n", i, actorNames[i].c_str());

        // get long descriptions
        string descsi = csv.get_value(3 + i, 2);
        actorDescs.push_back(descsi);
        printf("Actor %3i desc: %s \n", i, actorDescs[i].c_str());

        // get capability/power, often on 0-100 scale
        string psi = csv.get_value(3 + i, 3);
        double pi = atof(psi.c_str());
        printf("Actor %3i power: %5.1f \n", i, pi);
        assert(0 <= pi); // zero weight is pointless, but not incorrect
        assert(pi < 1E8); // no real upper limit, so this is just a sanity-check
        cap(i, 0) = pi;


        cout << endl << flush;

    } // loop over actors, i


    // get issue names
    auto dNames = vector<string>();
    for (unsigned int j = 0; j < numDim; j++) {
        string insi = csv.get_value(2, 4 + 2 * j);
        dNames.push_back(insi);
        printf("Dimension %2i: %s \n", j, dNames[j].c_str());
    }
    cout << endl;

    // get position/salience data
    auto pos = KMatrix(numActor, numDim);
    auto sal = KMatrix(numActor, numDim);
    for (unsigned int i = 0; i < numActor; i++) {
        double salI = 0.0;
        for (unsigned int j = 0; j < numDim; j++) {
            string posSIJ = csv.get_value(3 + i, 4 + 2 * j);
            double posIJ = atof(posSIJ.c_str());
            printf("pos[%3i , %3i] =  %5.3f \n", i, j, posIJ);
            cout << flush;
            if ((posIJ < 0.0) || (+100.0 < posIJ)) { // lower and upper limit
                errBuff = newChars(100);
                sprintf(errBuff, "SMPModel::readCSV: Out-of-bounds position for actor %i on dimension %i:  %f",
                        i, j, posIJ);
                throw(KException(errBuff));
            }
            assert(0.0 <= posIJ);
            assert(posIJ <= 100.0);
            pos(i, j) = posIJ;

            string salSIJ = csv.get_value(3 + i, 5 + 2 * j);
            double salIJ = atof(salSIJ.c_str());
            //printf("sal[%3i , %3i] = %5.3f \n", i, j, salIJ);
            //cout << flush;
            if ((salIJ < 0.0) || (+100.0 < salIJ)) { // lower and upper limit
                errBuff = newChars(100);
                sprintf(errBuff, "SMPModel::readCSV: Out-of-bounds salience for actor %i on dimension %i:  %f",
                        i, j, salIJ);
                throw(KException(errBuff));
            }
            assert(0.0 <= salIJ);
            salI = salI + salIJ;
            //printf("sal[%3i] = %5.3f \n", i, salI);
            //cout << flush;
            if (+100.0 < salI) { // upper limit: no more than 100% of attention to all issues
                errBuff = newChars(100);
                sprintf(errBuff,
                        "SMPModel::readCSV: Out-of-bounds total salience for actor %i:  %f",
                        i, salI);
                throw(KException(errBuff));
            }
            assert(salI <= 100.0);
            sal(i, j) = (double)salIJ;
            //cout << endl << flush;
        }
    }

    cout << "Position matrix:" << endl;
    pos.printf("%5.1f  ");
    cout << endl << endl << flush;
    cout << "Salience matrix:" << endl;
    sal.printf("%5.1f  ");
    cout << endl << flush;

    // get them into the proper internal scale:
    pos = pos / 100.0;
    sal = sal / 100.0;

    // now that it is read and verified, use the data
    auto sm0 = SMPModel::initModel(actorNames, actorDescs, dNames, cap, pos, sal, rng);
    return sm0;
}

SMPModel * SMPModel::initModel(vector<string> aName, vector<string> aDesc, vector<string> dName,
                               KMatrix cap, KMatrix pos, KMatrix sal, PRNG * rng) {
    SMPModel * sm0 = new SMPModel(rng);
    SMPState * st0 = new SMPState(sm0);
    st0->step = [st0]() {
        return st0->stepBCN();
    };
    sm0->addState(st0);


    const unsigned int na = aName.size();
    const unsigned int nd = dName.size();


    for (auto dn : dName) {
        sm0->addDim(dn);
    }

    for (unsigned int i = 0; i < na; i++) {
        auto ai = new SMPActor(aName[i], aDesc[i]);
        ai->sCap = cap(i, 0);
        ai->vSal = KMatrix(nd, 1);
        auto vpi = new VctrPstn(nd, 1);
        for (unsigned int j = 0; j < nd; j++) {
            ai->vSal(j, 0) = sal(i, j);
            (*vpi)(j, 0) = pos(i, j);
        }
        sm0->addActor(ai);
        st0->addPstn(vpi);
    }

    return sm0;
}



}; // end of namespace

// --------------------------------------------
// Copyright KAPSARC. Open source MIT License.
// --------------------------------------------