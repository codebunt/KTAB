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
// -------------------------------------------------

#include "modelwindow.h"

ModelFrame::ModelFrame(QObject *parent)
{
    frameMainLayout = new QGridLayout(this);
    initializeFrameLayout();
    setLayout(frameMainLayout);

}

ModelFrame::~ModelFrame()
{

}

void ModelFrame::initializeModelParameters()
{
    QGridLayout *parametersGLayout = new QGridLayout;

    victProbModelComboBox = new QComboBox;
    pCEModelComboBox = new QComboBox;
    stateTransitionsComboBox = new QComboBox;
    votingRuleComboBox = new QComboBox;
    bigRAdjustComboBox = new QComboBox;
    bigRRangeComboBox = new QComboBox;
    thirdPartyCommitComboBox = new QComboBox;
    interVecBrgnComboBox = new QComboBox;
    bargnModelComboBox = new QComboBox;

    //victPro
    modelTips.append("Controls the rate at which the probability of a coalition supporting "
                     "\nan option winning against the coalition proposing it increases as the "
                     "\nstrength ratios increase");
    //pce
    modelTips.append("Controls the type of probabilistic condorcet election used");
    //state
    modelTips.append("Controls how the winning bargain in an actor's queue is chosen among all bargains");
    //  voting rule
    modelTips.append("Controls how the amount of influence an actor will exert between two options "
                     "\ndepends on the perceived difference in utilities");
    //bigRadjust
    modelTips.append("Controls how accurately actor i is able to estimate, relative to an anchor of its own risk attitude,"
                     "\nthe risk attitude of actor j (which is known to the model)");
    //bigRRange
    modelTips.append("Controls actors' risk tolerances, and hence the curvature of their utility functions");
    //thirdparty
    modelTips.append("Controls how committed a third party actor k is in a challenge between "
                     "actors i and j");
    //interVic
    modelTips.append("Controls how proposed positions are interpolated between the positions of actor i and"
                     " j in a bargain");
    //bargnmodel
    modelTips.append("Controls from which actor's perspective the probability of success is\n used to "
                     "interpolate bargains");

    QLabel *victProbLabel = new QLabel(" VictoryProbModel ");
    victProbLabel->setToolTip(modelTips.at(0));
    victProbLabel->setFrameStyle(QFrame::StyledPanel);
    victProbLabel->setAlignment(Qt::AlignHCenter);

    QLabel *pCELabel =  new QLabel(" ProbCondorcetElection ");
    pCELabel->setToolTip(modelTips.at(1));
    pCELabel->setFrameStyle(QFrame::StyledPanel);
    pCELabel->setAlignment(Qt::AlignHCenter);

    QLabel *stateTransitionsLabel =  new QLabel(" StateTransition ");
    stateTransitionsLabel->setToolTip(modelTips.at(2));
    stateTransitionsLabel->setFrameStyle(QFrame::StyledPanel);
    stateTransitionsLabel->setAlignment(Qt::AlignHCenter);

    QLabel *votingRuleLabel =  new QLabel(" VotingRule ");
    votingRuleLabel->setToolTip(modelTips.at(3));
    votingRuleLabel->setFrameStyle(QFrame::StyledPanel);
    votingRuleLabel->setAlignment(Qt::AlignHCenter);

    QLabel *bigRAdjustLabel =  new QLabel(" BigRAdjust ");
    bigRAdjustLabel->setToolTip(modelTips.at(4));
    bigRAdjustLabel->setFrameStyle(QFrame::StyledPanel);
    bigRAdjustLabel->setAlignment(Qt::AlignHCenter);

    QLabel *bigRRangeLabel =  new QLabel(" BigRRange ");
    bigRRangeLabel->setToolTip(modelTips.at(5));
    bigRRangeLabel->setFrameStyle(QFrame::StyledPanel);
    bigRRangeLabel->setAlignment(Qt::AlignHCenter);

    QLabel *thirdPartyCommitLabel =  new QLabel(" ThirdPartyCommit ");
    thirdPartyCommitLabel->setToolTip(modelTips.at(6));
    thirdPartyCommitLabel->setFrameStyle(QFrame::StyledPanel);
    thirdPartyCommitLabel->setAlignment(Qt::AlignHCenter);

    QLabel *interVecBrgnLabel =  new QLabel(" InterVecBrgn ");
    interVecBrgnLabel->setToolTip(modelTips.at(7));
    interVecBrgnLabel->setFrameStyle(QFrame::StyledPanel);
    interVecBrgnLabel->setAlignment(Qt::AlignHCenter);

    QLabel *bargnModelLabel =  new QLabel(" BargnModel ");
    bargnModelLabel->setToolTip(modelTips.at(8));
    bargnModelLabel->setFrameStyle(QFrame::StyledPanel);
    bargnModelLabel->setAlignment(Qt::AlignHCenter);

    modelParameters.clear();
    modelParametersList << "VictoryProbModel" << "ProbCondorcetElection" << "StateTransition" << "VotingRule"
                        << "BigRAdjust" << "BigRRange" << "ThirdPartyCommit" << "InterVecBrgn" << "BargnModel";

    victList << "Linear" << "Square" << "Quartic" << "Octic" << "Binary";
    modelParameters.append(victList);

    pceList << "Conditional" << "MarkovIncentive" << "MarkovUniform";
    modelParameters.append(pceList);

    stateList << "Deterministic" << "Stochastic";
    modelParameters.append(stateList);

    votingList << "Binary" << "PropBin"<< "Prop"<< "PropCbc"<< "Cubic"<< "ASymProsp";
    modelParameters.append(votingList);

    bigAdjList << "None" << "OneThird" << "Half"<< "TwoThirds"<< "Full";
    modelParameters.append(bigAdjList);

    bigRangeList << "Min" << "Mid" << "Max";
    modelParameters.append(bigRangeList);

    thridpartyList << "NoCommit" << "SemiCommit" << "FullCommit";
    modelParameters.append(thridpartyList);

    interVecList << "S1P1" << "S2P2" << "S2PMax";
    modelParameters.append(interVecList);

    bargnList << "InitOnlyInterp" << "InitRcvrInterp" << "PWCompInterp";
    modelParameters.append(bargnList);

    QStringList tooltipList;
    QVBoxLayout * v1 = new QVBoxLayout;
    victProbModelComboBox->addItems(victList);
    victProbModelComboBox->setToolTip(modelTips.at(0));

    tooltipList.append("A 2:1 ratio gives a probability of 2/3 to the "
                       "\nstronger coalition (default)");
    tooltipList.append("A 2:1 ratio gives a probability of 4/5 to the stronger coalition");
    tooltipList.append("A 2:1 ratio gives a probability of 16/17 to the stronger coalition");
    tooltipList.append("A 2:1 ratio gives a probability of 256/257 to the stronger coalition");
    tooltipList.append("Any significant percentage difference gives a probability of 1 to "
                       "\nthe stronger coalition");

    victProbModelComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    victProbModelComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    victProbModelComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);
    victProbModelComboBox->setItemData(3,tooltipList.at(3), Qt::ToolTipRole);
    victProbModelComboBox->setItemData(4,tooltipList.at(4), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v1->addWidget(victProbLabel,0,Qt::AlignBottom);
    v1->addWidget(victProbModelComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v1,0,0);

    QVBoxLayout * v2 = new QVBoxLayout;
    pCEModelComboBox->addItems(pceList);
    pCEModelComboBox->setToolTip(modelTips.at(1));

    tooltipList.append("PCE uses single-step conditional probabilities (default)");
    tooltipList.append("PCE uses a Markov process in which challenge probabilities are\n proportional"
                       " to promoting influence");
    tooltipList.append("PCE uses a Markov process in which challenge \nprobabilities are "
                       "uniform");

    pCEModelComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    pCEModelComboBox->setItemData(1,tooltipList.at(1),Qt::ToolTipRole);
    pCEModelComboBox->setItemData(2,tooltipList.at(2),Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v2->addWidget(pCELabel,0,Qt::AlignBottom);
    v2->addWidget(pCEModelComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v2,0,1);

    QVBoxLayout * v3 = new QVBoxLayout;
    stateTransitionsComboBox->addItems(stateList);
    stateTransitionsComboBox->setToolTip(modelTips.at(2));

    tooltipList.append("The bargain which has the strongest coalition, and hence the highest"
                       "\n probability of winning, wins (default)");
    tooltipList.append("The probability of winning for each bargain is proportional to its "
                       "\nrelative coalition strength");

    stateTransitionsComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    stateTransitionsComboBox->setItemData(1,tooltipList.at(1),Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v3->addWidget(stateTransitionsLabel,0,Qt::AlignBottom);
    v3->addWidget(stateTransitionsComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v3,0,2);

    QVBoxLayout * v4 = new QVBoxLayout;
    votingRuleComboBox->addItems(votingList);
    votingRuleComboBox->setToolTip(modelTips.at(3));

    tooltipList.append("The actor exerts all influence, regardless of the difference in utilities");
    tooltipList.append("The vote is proportional to the weighted average of Prop (80%) and Binary (20%)");
    tooltipList.append("The vote is linearly proportional to the difference in utilities (default)");
    tooltipList.append("The vote is proportional to the average of Prop and Cubic");
    tooltipList.append("The vote is proportional to the cubed difference in utilities");
    tooltipList.append("Influence is exerted asymmetrically: It is proportional to the difference"
                       "\nof utilities if negative (a loss in utility)."
                       "\nIt is proportional to 2/3 of the difference, if positive (a gain in utility)");

    votingRuleComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    votingRuleComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    votingRuleComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);
    votingRuleComboBox->setItemData(3,tooltipList.at(3), Qt::ToolTipRole);
    votingRuleComboBox->setItemData(4,tooltipList.at(4), Qt::ToolTipRole);
    votingRuleComboBox->setItemData(5,tooltipList.at(5), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v4->addWidget(votingRuleLabel,0,Qt::AlignBottom);
    v4->addWidget(votingRuleComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v4,1,0);

    QVBoxLayout * v5 = new QVBoxLayout;
    bigRAdjustComboBox->addItems(bigAdjList);
    bigRAdjustComboBox->setToolTip(modelTips.at(4));

    tooltipList.append("Actor i judges actor j's risk attitude as being identical to its risk attitude");
    tooltipList.append("Actor i estimates actor j's risk attitude by interpolating between them, such that its estimate"
                       "\nis closer (2/3 anchored, 1/3 adjusted) to its risk attitude (default)");
    tooltipList.append("Actor i estimates actor j's risk attitude by interpolating midway between its "
                       "\nrisk attitude and actor j's actual risk attitude");
    tooltipList.append("Actor i estimates actor j's risk attitude by interpolating between them, such that "
                       "\nits estimate is closer (1/3 anchored, 2/3 adjusted) to actor j's risk attitude");
    tooltipList.append("Actor i judges actor j's risk attitude correctly");

    bigRAdjustComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    bigRAdjustComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    bigRAdjustComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);
    bigRAdjustComboBox->setItemData(3,tooltipList.at(3), Qt::ToolTipRole);
    bigRAdjustComboBox->setItemData(4,tooltipList.at(4), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v5->addWidget(bigRAdjustLabel,0,Qt::AlignBottom);
    v5->addWidget(bigRAdjustComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v5,1,1);

    QVBoxLayout * v6 = new QVBoxLayout;
    bigRRangeComboBox->addItems(bigRangeList);
    bigRRangeComboBox->setToolTip(modelTips.at(5));

    tooltipList.append("Sets risk tolerances in the range [0,1] such that actors with the most probable"
                       "\nposition are perfectly risk averse (1), while actors holding the least probable "
                       "\nposition are perfectly risk tolerant (0)");
    tooltipList.append("Sets risk tolerances in the range [-½,1] such that actors with the most probable "
                       "\nposition are perfectly risk averse (1), while actors holding the least probable"
                       "\nposition are somewhat risk seeking, with an aversion of -½ (default)If the coalition "
                       "\nactor k has joined loses, k must take the position of the winning coalition; otherwise"
                       "\nit does not need to change position (default)");
    tooltipList.append("Sets risk tolerances in the range [-1,1] such that actors with the most probable position"
                       "\nare perfectly risk averse (1), while actors holding the least probable position are "
                       "\nperfectly risk seeking (-1)");

    bigRRangeComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    bigRRangeComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    bigRRangeComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v6->addWidget(bigRRangeLabel,0,Qt::AlignBottom);
    v6->addWidget(bigRRangeComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v6,1,2);

    QVBoxLayout * v7 = new QVBoxLayout;
    thirdPartyCommitComboBox->addItems(thridpartyList);
    thirdPartyCommitComboBox->setToolTip(modelTips.at(6));

    tooltipList.append("No matter which coalition actor k joins (i or j), actor k never changes position");
    tooltipList.append("If the coalition joined by actor k loses, k must take the position of the winning"
                       "\ncoalition; otherwise it does not need to change position (default)");
    tooltipList.append("Actor k is fully committed to the coalition it joins, and must adopt the position "
                       "\nof the winning coalition");

    thirdPartyCommitComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    thirdPartyCommitComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    thirdPartyCommitComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v7->addWidget(thirdPartyCommitLabel,0,Qt::AlignBottom);
    v7->addWidget(thirdPartyCommitComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v7,2,0);

    QVBoxLayout * v8 = new QVBoxLayout;
    interVecBrgnComboBox->addItems(interVecList);
    interVecBrgnComboBox->setToolTip(modelTips.at(7));

    tooltipList.append("Proposed postions for each actor are computed as a weighted average of their "
                       "\ncurrent positions, where the weights are the products of salience and probability of success");
    tooltipList.append("Proposed postions for each actor are computed as a weighted average of their current"
                       "\npositions, where the weights are the squared products of salience*probability of "
                       "\nsuccess (default)");
    tooltipList.append("Proposed postions for each actor are computed as asymmetric shifts from their current"
                       "\npositions, which is a function of squared salience and truncated difference in "
                       "\nprobability of success");

    interVecBrgnComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    interVecBrgnComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    interVecBrgnComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v8->addWidget(interVecBrgnLabel,0,Qt::AlignBottom);
    v8->addWidget(interVecBrgnComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v8,2,1);

    QVBoxLayout * v9 = new QVBoxLayout;
    bargnModelComboBox->addItems(bargnList);
    bargnModelComboBox->setToolTip(modelTips.at(8));

    tooltipList.append("Bargains are only computed from the initiating actor's perspective (default)");
    tooltipList.append("Bargains are computed from the perspective of both the initiating actor and receiving actor");
    tooltipList.append("Bargains are computed as an effective power-weighted average of both actor's perspectives");

    bargnModelComboBox->setItemData(0,tooltipList.at(0), Qt::ToolTipRole);
    bargnModelComboBox->setItemData(1,tooltipList.at(1), Qt::ToolTipRole);
    bargnModelComboBox->setItemData(2,tooltipList.at(2), Qt::ToolTipRole);

    parameterTips.append(tooltipList);
    tooltipList.clear();

    v9->addWidget(bargnModelLabel,0,Qt::AlignBottom);
    v9->addWidget(bargnModelComboBox,0,Qt::AlignTop);
    parametersGLayout->addLayout(v9,2,2);

    modelParaGridLayout->addLayout(parametersGLayout,0,0);

}

void ModelFrame::initializeModelControls()
{
    controlsLayout = new QGridLayout;
    parameterName = new QComboBox;
    parameterName->addItems(modelParametersList);
    for(int index = 0; index < modelParametersList.length();++index)
    {
        parameterName->setItemData(index,modelTips.at(index),Qt::ToolTipRole);
    }
    connect(parameterName,SIGNAL(currentIndexChanged(int)),this,SLOT(parameterChanged(int)));

    addAllCheckBox = new QCheckBox("Add All");
    connect(addAllCheckBox,SIGNAL(clicked(bool)),this,SLOT(addAllClicked(bool)));

    QPushButton * addSpecPushButton = new QPushButton("Add Specification");
    connect(addSpecPushButton,SIGNAL(clicked(bool)),this,SLOT(addSpecClicked(bool)));

    parmetersFrame = new QFrame;
    controlsLayout->addWidget(parameterName,0,0);
    controlsLayout->addWidget(parmetersFrame,1,0);
    controlsLayout->addWidget(addAllCheckBox,2,0);
    controlsLayout->addWidget(addSpecPushButton,3,0);

    modelControlsGridLayout->addLayout(controlsLayout,0,0);
}

void ModelFrame::initializeModelSpecifications()
{
    specsListView = new QListView;
    specsListView->setAutoScroll(true);
    connect(specsListView,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(modelListViewContextMenu(QPoint)));
    specsListView->setContextMenuPolicy(Qt::CustomContextMenu);

    specsListModel = new QStandardItemModel();

    specsListView->setModel(specsListModel);

    modelSpecsGridLayout->addWidget(specsListView);
}

void ModelFrame::initializeFrameLayout()
{
    QFrame * topFrame = new QFrame;
    QGridLayout * gridL = new QGridLayout;

    QSplitter *splitterH = new  QSplitter(Qt::Horizontal,this);
    QSplitter *splitterV = new  QSplitter(Qt::Vertical,this);

    modelParaFrame = new QFrame;
    modelControlsFrame = new  QFrame;
    modelSpecsFrame = new QFrame;

    modelParaGridLayout = new QGridLayout;
    modelControlsGridLayout = new QGridLayout;
    modelSpecsGridLayout = new QGridLayout;

    modelParaFrame->setFrameStyle(QFrame::StyledPanel);
    modelControlsFrame->setFrameStyle(QFrame::StyledPanel);

    initializeModelParameters();
    initializeModelControls();
    parameterChanged(0); // default parameter init
    initializeModelSpecifications();

    modelParaFrame->setLayout(modelParaGridLayout);
    modelControlsFrame->setLayout(modelControlsGridLayout);
    modelSpecsFrame->setLayout(modelSpecsGridLayout);

    splitterH->addWidget(modelParaFrame);
    splitterH->addWidget(modelControlsFrame);
    splitterH->setChildrenCollapsible(false);

    gridL->addWidget(splitterH);
    topFrame->setLayout(gridL);

    splitterV->addWidget(topFrame);
    splitterV->addWidget(modelSpecsFrame);
    splitterV->setChildrenCollapsible(false);

    frameMainLayout->addWidget(splitterV);
    setFrameStyle(QFrame::StyledPanel);

}

void ModelFrame::parameterChanged(int index)
{
    parameterName->setToolTip(modelTips.at(index));
    parametersCheckBoxList.clear();
    parmetersFrame->close();
    controlsLayout->removeWidget(parmetersFrame);
    if(parmetersFrame != nullptr)
    {
        delete parmetersFrame;
    }
    parmetersFrame = new QFrame;
    parmetersFrame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout* parametersLayout = new QVBoxLayout;

    for(int i = 0; i < modelParameters.at(index).length();++i)
    {
        QCheckBox * parameters = new QCheckBox;
        parameters->setText(modelParameters.at(index).at(i));
        parameters->setToolTip(parameterTips.at(index).at(i));
        parametersLayout->addWidget(parameters);
        parametersCheckBoxList.append(parameters);
    }
    parmetersFrame->setLayout(parametersLayout);
    controlsLayout->addWidget(parmetersFrame,1,0);
    addAllCheckBox->setChecked(false);

}

void ModelFrame::addAllClicked(bool bl)
{
    for(int para = 0 ; para < parametersCheckBoxList.length(); ++ para)
    {
        parametersCheckBoxList.at(para)->setChecked(bl);
    }
}

void ModelFrame::addSpecClicked(bool bl)
{
    int count = 0;
    int specsCount = specsListModel->rowCount();
    QString specification;
    specification.append(parameterName->currentText()).append("=(");
    QVector <QString> params;
    for(int para = 0 ; para < parametersCheckBoxList.length(); ++ para)
    {
        if(true==parametersCheckBoxList.at(para)->isChecked())
        {
            params.append(parametersCheckBoxList.at(para)->text());
            specification.append(parametersCheckBoxList.at(para)->text()).append(",");
            count++;
        }
    }
    if(!params.isEmpty())
    {
        modelParaLHS.append(parameterName->currentText());
        modelParaRHS.append(params);
    }
    specification.append(")");
    specification.remove(",)").append(")");
    qDebug()<<modelParaRHS << modelParaLHS;

    if(count>0)
    {
        QStandardItem *item = new QStandardItem(specification);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setEditable(false);
        specsListModel->setItem(specsListModel->rowCount(),item);
        specsListView->scrollToBottom();
    }

    if(specsCount != specsListModel->rowCount())
    {
        for(int i = 0 ; i < specsListModel->rowCount(); ++i)
        {
            qDebug()<<specsListModel->item(i)->data().toString();
        }
        QPair<DataValues,SpecsData> spec;
        spec.first.append(modelParaLHS.at(modelParaLHS.count()-1));
        spec.second.append(modelParaRHS.at(modelParaRHS.count()-1));
        emit specificationNew(specsListModel->item(specsListModel->rowCount()-1)->text(),spec,0);//0 == model
    }
}

void ModelFrame::listViewClicked()
{
    for(int index=0; index < specsListModel->rowCount();++index)
    {
        QStandardItem * item = specsListModel->item(index);
        if (item->data(Qt::CheckStateRole).toBool() == true)   // Item checked, remove
        {
            removeSpecificationModel(index,0,modelParaLHS.at(index)); // 0 == model type
            specsListModel->removeRow(index);
            modelParaLHS.removeAt(index);
            modelParaRHS.removeAt(index);
            index = index -1; // index changed to current row, deletion of item changes list index
        }
    }
}

void ModelFrame::modelListViewContextMenu(QPoint pos)
{
    if(specsListView->model()->rowCount()>0)
    {
        QMenu *menu = new QMenu(this);
        menu->addAction("Remove Selected Items", this, SLOT(listViewClicked()));
        menu->addAction("Remove All Items", this, SLOT(listViewRemoveAllClicked()));
        menu->popup(specsListView->mapToGlobal(pos));
    }
}

void ModelFrame::listViewRemoveAllClicked()
{
    for(int index=0; index < specsListModel->rowCount();++index)
    {
        removeSpecificationModel(index,0,modelParaLHS.at(index)); // 0 == model type
        specsListModel->removeRow(index);
        modelParaLHS.removeAt(index);
        modelParaRHS.removeAt(index);
        index = index -1; // index changed to current row, deletion of item changes list index
    }
}


