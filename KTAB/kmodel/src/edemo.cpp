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
// Demonstrate some very basic functionality of
// the fundamental KBase namespace, from classes
// and functions in the MDemo namespace.
// --------------------------------------------

#include "edemo.h" 
#include "emodel.cpp" 
#include <easylogging++.h>



namespace MDemo { // a namespace of which M, A, P, S know nothing
using std::function;
using std::get;
using KBase::ReportingLevel;
using KBase::PRNG;
using KBase::KMatrix;

using KBase::Actor;
using KBase::Model;
using KBase::Position;
using KBase::State;
using KBase::VotingRule;
using KBase::KException;

// --------------------------------------------

TwoDPoint::TwoDPoint() {
  // default is set in class definition: (0,0)
}

TwoDPoint::TwoDPoint(unsigned int a, unsigned int b) {
  x = a;
  y = b;
}


TwoDPoint::~TwoDPoint() {
  x = 0;
  y = 0;
}

vector<TwoDPoint> theta2D() {
  vector<TwoDPoint> rslt = {};
  for (unsigned int i = 0; i <= 100; i++) {
    for (unsigned int j = 0; j <= 100; j++) {
      auto p = TwoDPoint(i, j);
      rslt.push_back(p);
    }
  }
  return rslt;
}
// --------------------------------------------


// lambda-bind the parameters and return a function that enumerates the options
function < vector<VBool>()> tbv(unsigned int nAct, unsigned int nBits, PRNG* rng) {

  const double fc = exp(log(2.0)*nBits);
  unsigned int ic = (unsigned int)(0.5 + fc);

  auto nFromBV = [nBits](VBool bv) {
    // given a bit vector, treat it as base-2 and calculate the corresponding integer
    unsigned int rm = 0;
    unsigned int m = 1;
    for (unsigned int j = 0; j < nBits; j++) {
      if (bv[j]) {
        rm = rm + m;
      }
      m = m * 2;
    }
    return rm;
  };

  auto rfn = [nFromBV, ic, nBits]() {
    vector<VBool> rbv = {};
    for (unsigned int i = 0; i < ic; i++) {
      auto vi = VBool();
      unsigned int m = i;
      for (unsigned int j = 0; j < nBits; j++) {
        vi.push_back(1 == (m & 1));
        m = m >> 1;
      }

      unsigned int ni = nFromBV(vi);
      if (i != ni) {
        throw KException("tbv: inaccurate ni");
      }

      rbv.push_back(vi);
    }
    return rbv;
  };

  auto normRows = [](const KMatrix& um1) {
    KMatrix um2 = um1;
    for (unsigned int i = 0; i < um1.numR(); i++) {
      double iMin = um1(i, 0);
      double iMax = um1(i, 0);
      for (unsigned int j = 0; j < um1.numC(); j++) {
        iMin = (um1(i, j) < iMin) ? um1(i, j) : iMin;
        iMax = (um1(i, j) > iMax) ? um1(i, j) : iMax;
      }
      if (iMax <= iMin) {
        throw KException("tbv: iMax must be more than iMin");
      }
      for (unsigned int j = 0; j < um1.numC(); j++) {
        um2(i, j) = (um1(i, j) - iMin) / (iMax - iMin);
      }
    }
    return um2;
  };

  auto smooth = [normRows, nAct, nBits, ic](const KMatrix& um1) {
    auto um2 = um1;
    if (nAct != um1.numR()) {
      throw KException("tbv: inaccurate number of rows in um1");
    }
    if (ic != um1.numC()) {
      throw KException("tbv: inaccurate number of columns in um1");
    }
    for (unsigned int i = 0; i < nAct; i++) { // for each row, i.e. each actor
      for (unsigned int j = 0; j < ic; j++) { // for each column, i.e. each vector
        double sumij = um1(i, j);
        for (unsigned int s = 0; s < nBits; s++) {
          unsigned int k = j ^ (1 << s); // flip the s-bit
          if (k >= ic) {
            throw KException("tbv: k must be less than ic");
          }
          sumij = sumij + um1(i, k);
        }
        um2(i, j) = sumij / (1.0 + nBits);
      }
    }

    // now rescale so each actor has max/min utility of 1/0
    um2 = normRows(um2);
    return um2;
  };

  // we create a completely random matrix of utilities, then add
  // structure by averaging each point with its n neighbors twice
  auto uMat1 = KMatrix::uniform(rng, nAct, ic, 0.0, 1.0);
  auto uMat2 = smooth(smooth(uMat1));
  uMat2.mPrintf("%.4f "); 
  return rfn;
}



function < vector<VBool>()> thetaBV(unsigned int n) {
  auto rfn = [n]() {
    vector<VBool> rbv = {};
    const double fc = exp(log(2.0)*n);
    unsigned int ic = (unsigned int)(0.5 + fc);
    for (unsigned int i = 0; i < ic; i++) {
      auto vi = VBool();
      unsigned int m = i;
      for (unsigned int j = 0; j < n; j++) {
        if (0 == (m % 2)) {
          vi.push_back(false);
        }
        else {
          vi.push_back(true);
        }
        m = m / 2;
      }
      rbv.push_back(vi);
    }
    return rbv;
  };
  return rfn;
}


// --------------------------------------------

void demoEMod(uint64_t s) {
  using KBase::EModel;
  LOG(INFO) << " ";
  LOG(INFO) << KBase::getFormattedString("demoEMod using PRNG seed:  %020llu", s);

  LOG(INFO) << "Creating EModel objects ... ";

  string n2D = "EModel-TwoDPoint";
  auto em2D = new EModel<TwoDPoint>(n2D, s);
  LOG(INFO) << KBase::getFormattedString("Populating %s ", n2D.c_str());
  em2D->enumOptions = theta2D;
  em2D->setOptions();
  LOG(INFO) << KBase::getFormattedString("Now have %u enumerated options", em2D->numOptions());
   
  string nBV = "EModel-VBool";
  EModel<VBool>* emBV = new EModel<VBool>( nBV, s); 
  LOG(INFO) << KBase::getFormattedString("Populating %s ", nBV.c_str());
  const unsigned int numActTBV = 17;
  const unsigned int numBitsTBV = 4;
  emBV->enumOptions = tbv(numActTBV, numBitsTBV, emBV->rng); //  thetaBV(4);
  emBV->setOptions(); 
  LOG(INFO) << KBase::getFormattedString("Now have %u enumerated options", emBV->numOptions());


  LOG(INFO) << "Deleting EModel objects ... ";
  delete em2D;
  delete emBV;
  return;
}


}; // end of namespace


// --------------------------------------------
// Copyright KAPSARC. Open source MIT License.
// --------------------------------------------
