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

#include <assert.h>
#include <iostream>

#include "kmodel.h"


namespace KBase {

using std::cout;
using std::endl;
using std::flush;
using std::get;
using std::tuple;


// --------------------------------------------

void Model::demoSQLite() {
    cout << endl << "Starting basic demo of SQLite in Model class" << endl;

    auto callBack = [](void *data, int numCol, char **stringFields, char **colNames) {
        for (int i = 0; i < numCol; i++) {
            printf("%s = %s\n", colNames[i], stringFields[i] ? stringFields[i] : "NULL");
        }
        printf("\n");
        return ((int)0);
    };

    sqlite3 *db = nullptr;
    char* zErrMsg = nullptr;
    int  rc;
    string sql;

    auto sOpen = [&db](unsigned int n) {
        int rc = sqlite3_open("test.db", &db);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            exit(0);
        }
        else {
            fprintf(stderr, "Opened database successfully (%i)\n", n);
        }
        return;
    };

    auto sExec = [callBack, &db, &zErrMsg](string sql, string msg) {
        int rc = sqlite3_exec(db, sql.c_str(), callBack, nullptr, &zErrMsg); // nullptr is the 'data' argument
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, msg.c_str());
        }
        return rc;
    };


    // Open database
    cout << endl << flush;
    sOpen(1);

    // Create SQL statement
    sql = "create table if not exists PETS("  \
          "ID INT PRIMARY KEY     NOT NULL," \
          "NAME           text    NOT NULL," \
          "AGE            int     NOT NULL," \
          "BREED         char(50)," \
          "COLOR         char(50) );";

    // Execute SQL statement
    rc = sExec(sql, "Created table successfully \n");
    sqlite3_close(db);


    cout << endl << flush;
    sOpen(2);

    // This SQL statement has one deliberate error.
    sql = "INSERT INTO PETS (ID, NAME, AGE, BREED, COLOR) "   \
          "VALUES (1, 'Alice', 6, 'Greyhound', 'Grey' ); "    \
          "INSERT INTO PETS (ID, NAME, AGE, BREED, COLOR) "   \
          "VALUES (2, 'Bob', 4, 'Newfoundland', 'Black' ); "  \
          "INSERT INTO PETS (ID, NAME, AGE, BREED, COLOR) "   \
          "VALUES (3, 'Carol', 7, 'Chihuahua', 'Tan' );"      \
          "INSERT INTO PETS (ID, NAME, AGE, SPECIES, COLOR) " \
          "VALUES (4, 'David', 5, 'Alsation', 'Mixed' );";
    \
    "INSERT INTO PETS (ID, NAME, AGE, SPECIES, COLOR) " \
    "VALUES (5, 'Ellie', 8, 'Rhodesian', 'Red' );";

    cout << "NB: This should get one planned SQL error at ID=4" << endl << flush;
    rc = sExec(sql, "Records inserted successfully \n");
    sqlite3_close(db);


    cout << endl << flush;
    sOpen(3);
    sql = "SELECT * from PETS where AGE>5;";
    rc = sExec(sql, "Records selected successfully\n");
    cout << "NB: ID=5 was never inserted due to planned SQL error at ID=4" << endl;
    sqlite3_close(db);


    cout << endl << flush;
    sOpen(4);
    sql = "DROP TABLE PETS;";
    rc = sExec(sql, "Dropped table successfully \n");
    sqlite3_close(db);

    return;
}



// note that the function to write to table #k must be kept
// synchronized with the result of createSQL(k) !
string Model::createSQL(unsigned int n) {

    string sql = "";
    assert(n < Model::NumTables);
    switch (n) {
    case 0:
        // position-utility table
        // the estimated utility to each actor of each other's position
        sql = "create table if not exists PosUtil ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Act_i	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util	REAL"\
              ");";
        break;

    case 1: // pos-vote table
        // estimated vote of each actor between each pair of positions
        sql = "create table if not exists PosVote ("  \
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
        sql = "create table if not exists PosProb ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_i	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL"\
              ");";
        break;

    case 3: // pos-equiv table. E(i)= lowest j s.t. Pos(i) ~ Pos(j). if j < i, it is not unique.
        sql = "create table if not exists PosEquiv ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Pos_i	INTEGER NOT NULL DEFAULT 0, "\
              "Eqv_j	INTEGER NOT NULL DEFAULT 0 "\
              ");";
        break;

    case 4:
        // h's estimate of the utility to k of i challenging j, preceded by intermediate values
        // U{^h}{_k} ( SQ ), current status quo positions
        // U{^h}{_k} ( i >  j ), i defeats j
        // U{^h}{_k} ( i :  j ), contest between i and j
        // U{^h}{_k} ( i => j ), i challenges j
        // Utilities are evaluated so that UtilSQ, UtilVict, UtilChlg, UtilContest,
        // UtilTPVict, UtilTPLoss are comparable, i.e. the differences are meaningful
        sql = "create table if not exists UtilChlg ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Aff_k	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Util_SQ	   	REAL    NOT NULL DEFAULT 0, "\
              "Util_Vict	REAL    NOT NULL DEFAULT 0, "\
              "Util_Cntst	REAL    NOT NULL DEFAULT 0, "\
              "Util_Chlg	REAL    NOT NULL DEFAULT 0  "\
              ");";
        break;

    case 5:
        // h's estimate that i will defeat j, including all third party contributions
        // P{^h}( i > j )
        sql = "create table if not exists ProbVict ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL"\
              ");";
        break;

    case 6:
        // h's estimates in a little 3-actor contest with no contribution from others:
        // P{^h}( ik > j )      probability ik defeats j,
        // U{^h}{_k} (ik > j)   utility to k of winning with i, over j
        // U{^h}{_k} (i > jk)   utility to k of losing to i, with j
        sql = "create table if not exists TP_Prob_Vict_Loss ("  \
              "Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
              "Turn_t	INTEGER NOT NULL DEFAULT 0, "\
              "Est_h	INTEGER NOT NULL DEFAULT 0, "\
              "Init_i	INTEGER NOT NULL DEFAULT 0, "\
              "ThrdP_k	INTEGER NOT NULL DEFAULT 0, "\
              "Rcvr_j	INTEGER NOT NULL DEFAULT 0, "\
              "Prob	REAL    NOT NULL DEFAULT 0, "\
              "Util_V	REAL    NOT NULL DEFAULT 0, "\
              "Util_L	REAL    NOT NULL DEFAULT 0  "\
              ");";
        break;

    case 7: { // short-name and long-description of actors
        char *sqlBuff = newChars(256);
        sprintf(sqlBuff, "create table if not exists ActorDescription ("  \
                "Scenario TEXT(%d) NOT NULL DEFAULT 'NoName', "\
                "Act_i	INTEGER NOT NULL DEFAULT 0, "\
                "Name	TEXT(%d) NOT NULL DEFAULT 'NoName', "\
                "Desc	TEXT(%d) NOT NULL DEFAULT 'NoName' "\
                ");", maxScenNameLen, maxActNameLen, maxActDescLen);
        sql = std::string(sqlBuff);
        delete sqlBuff;
        sqlBuff = nullptr;
    }
    break;

	case 8: // Bargain table
		sql = "create table if not exists Bargn ("  \
			"Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
			"Turn_t	INTEGER NOT NULL DEFAULT 0, "\
			"Bargn_i INTEGER NOT NULL DEFAULT 0, "\
			"Brgn_Act_i INTEGER NOT NULL DEFAULT 0, "\
			"Init_Act_i INTEGER NOT NULL DEFAULT 0, "\
			"Recd_Act_i INTEGER NOT NULL DEFAULT 0, "\
			"Value REAL NOT NULL DEFAULT 0.0, "\
			"Init_Prob REAL NULL DEFAULT 0, "\
			"Init_Seld	BOOLEAN NULL ,"\
			"Recd_Prob REAL NULL DEFAULT 0, "\
			"Recd_Seld	BOOLEAN NULL"\
			");";
			break;
	case 9:  //
			 // BargnValu table
		sql = "create table if not exists BargnValu ("  \
			"Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
			"Turn_t	INTEGER NOT NULL DEFAULT 0, "\
			"Bargn_i INTEGER NOT NULL DEFAULT 0, "\
			"Dim_k INTEGER NOT NULL DEFAULT 0, "\
			"Init_Coord	REAL NULL DEFAULT 0.0,"\
			"Recd_Coord	REAL"\
			");";
		break;
		 
	case 10:  // BargnUtil table creation
		sql = "create table if not exists BargnUtil ("  \
			"Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
			"Turn_t	INTEGER NOT NULL DEFAULT 0, "\
			"Bargn_i    INTEGER NOT NULL	DEFAULT 0, "\
			"Act_i 	INTEGER NOT NULL DEFAULT 0, "\
			"Util	REAL"\
			");";
		break;

	case 11:  // BargnVote table creation
		sql = "create table if not exists BargnVote ("  \
			"Scenario	TEXT NOT NULL DEFAULT 'NoName', "\
			"Turn_t	INTEGER NOT NULL DEFAULT 0, "\
			"Bargn_i  INTEGER NOT NULL DEFAULT 0, "\
			"Bargn_j INTEGER NOT NULL DEFAULT 0, "\
			"Act_i 	INTEGER NOT NULL DEFAULT 0, "\
			"Vote	REAL"\
			");";
		break;

    default:
        throw(KException("Model::createTableSQL unrecognized table number"));
    }

    return sql;
}


void Model::sqlAUtil(unsigned int t) {
    assert(nullptr != smpDB);
    assert(t < history.size());
    State* st = history[t];
    assert(nullptr != st);
    assert(numAct == st->aUtil.size());

    // I don't like passing 'this' into lambda-functions,
    // so I copy the pointer into a local variable I can pass in
    // without exposing everything in 'this'. If this were a big,
    // mission-critical RDBMS, rather than a 1-off record of this run,
    // doing so might be disasterous in case the system crashed before
    // things were cleaned up.
    sqlite3 * db = smpDB;
    smpDB = nullptr;

    char* zErrMsg = nullptr;
    auto sqlBuff = newChars(200);
    sprintf(sqlBuff,
            "INSERT INTO PosUtil (Scenario, Turn_t, Est_h, Act_i, Pos_j, Util) VALUES ('%s', ?1, ?2, ?3, ?4, ?5)",
            scenName.c_str());
    const char* insStr = sqlBuff;
    sqlite3_stmt *insStmt;
    sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
    // Prepared statements cache the execution plan for a query after the query optimizer has
    // found the best plan, so there is no big gain with simple insertions.
    // What makes a huge difference is bundling a few hundred into one atomic "transaction".
    // For this case, runtime droped from 62-65 seconds to 0.5-0.6 (vs. 0.30-0.33 with no SQL at all).

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    for (unsigned int h = 0; h < numAct; h++) { // estimator is h
        KMatrix uij = st->aUtil[h]; // utility to actor i of the position held by actor j
        for (unsigned int i = 0; i < numAct; i++) {
            for (unsigned int j = 0; j < numAct; j++) {
                int rslt = 0;
                rslt = sqlite3_bind_int(insStmt, 1, t);
                assert(SQLITE_OK == rslt);
                rslt = sqlite3_bind_int(insStmt, 2, h);
                assert(SQLITE_OK == rslt);
                rslt = sqlite3_bind_int(insStmt, 3, i);
                assert(SQLITE_OK == rslt);
                rslt = sqlite3_bind_int(insStmt, 4, j);
                assert(SQLITE_OK == rslt);
                rslt = sqlite3_bind_double(insStmt, 5, uij(i, j));
                assert(SQLITE_OK == rslt);
                rslt = sqlite3_step(insStmt);
                assert(SQLITE_DONE == rslt);
                sqlite3_clear_bindings(insStmt);
                assert(SQLITE_DONE == rslt);
                rslt = sqlite3_reset(insStmt);
                assert(SQLITE_OK == rslt);
            }
        }
    }
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
    printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);

    delete sqlBuff;
    sqlBuff = nullptr;

    smpDB = db; // give it the new pointer

    return;
}
// populates record for table PosEquiv for each step of
// module run
void Model::sqlPosEquiv(unsigned int t)
{
    // Check database intact
    assert(nullptr != smpDB);
    assert(t < history.size());
    State* st = history[t];
    assert(nullptr != st);
    sqlite3 * db = smpDB;
    smpDB = nullptr;
    // In case of error
    char* zErrMsg = nullptr;
    auto sqlBuff = newChars(200);
    sprintf(sqlBuff,
            "INSERT INTO PosEquiv (Scenario, Turn_t, Pos_i, Eqv_j) VALUES ('%s', ?1, ?2, ?3)",
            scenName.c_str());
    const char* insStr = sqlBuff;
    sqlite3_stmt *insStmt;
    sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
    // Start inserting record
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    for (unsigned int i = 0; i < numAct; i++) {
        // calculate the equivalance
        int je = numAct + 1;
        for (unsigned int j = 0; j < numAct && je > numAct; j++) {
            if (st->equivNdx(i, j)) {
                je = j;
            }
        }
        int rslt = 0;
        rslt = sqlite3_bind_int(insStmt, 1, t);
        assert(SQLITE_OK == rslt);
        rslt = sqlite3_bind_int(insStmt, 2, i);
        assert(SQLITE_OK == rslt);
        rslt = sqlite3_bind_int(insStmt, 3, je);
        assert(SQLITE_OK == rslt);
        rslt = sqlite3_step(insStmt);
        assert(SQLITE_DONE == rslt);
        sqlite3_clear_bindings(insStmt);
        assert(SQLITE_DONE == rslt);
        rslt = sqlite3_reset(insStmt);
        assert(SQLITE_OK == rslt);
    }
    // end databse transaction
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
    printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);
    delete sqlBuff;
    sqlBuff = nullptr;
    smpDB = db;
    return;
}

 
void Model::sqlUodateBargainTable (unsigned int t,   double IntProb, int Init_Seld, double Recd_Prob,int Recd_Seld,  int Brgn_Act_i)
{


	// initiate the database
	sqlite3 * db = smpDB;

	// Error message in case
	char* zErrMsg = nullptr;
	auto sqlBuff = newChars(200);
	// prepare the sql statement to insert
	sprintf(sqlBuff, "UPDATE Bargn SET Init_Prob = ?1, Init_Seld = ?2, Recd_Prob = ?3, Recd_Seld= ?4 WHERE (Brgn_Act_i = ?5 ) and (?6 = Turn_t)");

	const char* insStr = sqlBuff;
	sqlite3_stmt *insStmt;
	sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
	// start for the transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

	int rslt = 0;
	
	//Bargn_i
	rslt = sqlite3_bind_double(insStmt, 1, IntProb);
	assert(SQLITE_OK == rslt);
	//Brgn_Act_i
	rslt = sqlite3_bind_int(insStmt, 2, Init_Seld);
	assert(SQLITE_OK == rslt);
	//Init_Act_i
	rslt = sqlite3_bind_double(insStmt, 3, Recd_Prob);
	assert(SQLITE_OK == rslt);
	//Recd_Act_i
	rslt = sqlite3_bind_int(insStmt, 4, Recd_Seld);
	assert(SQLITE_OK == rslt);

	// Bargain actor
	rslt = sqlite3_bind_int(insStmt, 5, Brgn_Act_i);
	assert(SQLITE_OK == rslt);

	//turn
	rslt = sqlite3_bind_int(insStmt, 6, t);
	assert(SQLITE_OK == rslt);
	rslt = sqlite3_step(insStmt);
	assert(SQLITE_DONE == rslt);
	sqlite3_clear_bindings(insStmt);
	assert(SQLITE_DONE == rslt);
	rslt = sqlite3_reset(insStmt);
	assert(SQLITE_OK == rslt);

	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
	printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);

	delete sqlBuff;
	sqlBuff = nullptr;

	smpDB = db;
}

void Model::sqlBargainEntries(unsigned int t, int bargainId, int Baragainer, int initiator, int receiver, double val)
{
	// initiate the database
	sqlite3 * db = smpDB;

	// Error message in case
	char* zErrMsg = nullptr;
	auto sqlBuff = newChars(200);
	// prepare the sql statement to insert
	sprintf(sqlBuff,
		"INSERT INTO Bargn (Scenario, Turn_t,Bargn_i, Brgn_Act_i,Init_Act_i, Recd_Act_i,Value) VALUES ('%s',?1, ?2, ?3, ?4,?5,?6)",
		scenName.c_str());

	const char* insStr = sqlBuff;
	sqlite3_stmt *insStmt;
	sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
	// start for the transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

	int rslt = 0;
	// Turn_t
	rslt = sqlite3_bind_int(insStmt, 1, t);
	assert(SQLITE_OK == rslt);
	//Bargn_i
	rslt = sqlite3_bind_int(insStmt, 2, bargainId);
	assert(SQLITE_OK == rslt);
	//Brgn_Act_i
	rslt = sqlite3_bind_int(insStmt, 3, Baragainer);
	assert(SQLITE_OK == rslt);
	//Init_Act_i
	rslt = sqlite3_bind_int(insStmt, 4, initiator);
	assert(SQLITE_OK == rslt);
	//Recd_Act_i
	rslt = sqlite3_bind_int(insStmt, 5, receiver);
	assert(SQLITE_OK == rslt);
	//Value
	rslt = sqlite3_bind_double(insStmt, 6, val);
	assert(SQLITE_OK == rslt);
	rslt = sqlite3_step(insStmt);
	assert(SQLITE_DONE == rslt);
	sqlite3_clear_bindings(insStmt);
	assert(SQLITE_DONE == rslt);
	rslt = sqlite3_reset(insStmt);
	assert(SQLITE_OK == rslt);

	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
	printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);

	delete sqlBuff;
	sqlBuff = nullptr;

	smpDB = db;
}
 
void Model::sqlBargainValue(unsigned int t, int Baragainer, int Dim, KBase::VctrPstn Coord)
{
	// initiate the database
	sqlite3 * db = smpDB;

	// Error message in case
	char* zErrMsg = nullptr;
	auto sqlBuff = newChars(200);
 
	// prepare the sql statement to insert
	sprintf(sqlBuff,
		"INSERT INTO BargnValu (Scenario, Turn_t,Bargn_i, Dim_k, Init_Coord,Recd_Coord) VALUES ('%s',?1, ?2, ?3, ?4,?5)",
		scenName.c_str());

	const char* insStr = sqlBuff;
	sqlite3_stmt *insStmt;
	sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
	// start for the transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);


	 
	int rslt = 0;
	// Turn_t
	rslt = sqlite3_bind_int(insStmt, 1, t);
	assert(SQLITE_OK == rslt);
	//Baragainer
	rslt = sqlite3_bind_int(insStmt, 2, Baragainer);
	assert(SQLITE_OK == rslt);
	//Dim_K
	rslt = sqlite3_bind_int(insStmt, 3, Dim);
	assert(SQLITE_OK == rslt);
	 
	//Init_Coord
	rslt = sqlite3_bind_double(insStmt, 4, Coord(0, 0));
	assert(SQLITE_OK == rslt);
	//Recd_Coord
	rslt = sqlite3_bind_double(insStmt, 5, Coord(1, 0));
	assert(SQLITE_OK == rslt);
 

	assert(SQLITE_OK == rslt);
	rslt = sqlite3_step(insStmt);
	assert(SQLITE_DONE == rslt);
	sqlite3_clear_bindings(insStmt);
	assert(SQLITE_DONE == rslt);
	rslt = sqlite3_reset(insStmt);
	assert(SQLITE_OK == rslt);
	
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
 

	delete sqlBuff;
	sqlBuff = nullptr;

	smpDB = db;
}

void Model::sqlBargainUtil(unsigned int t, int Bargn_i,  KBase::KMatrix Util_mat)
{
	// initiate the database
	sqlite3 * db = smpDB;

	// Error message in case
	char* zErrMsg = nullptr;
	auto sqlBuff = newChars(200);

	int Util_mat_row = Util_mat.numR();
	int Util_mat_col = Util_mat.numC();

 
	// prepare the sql statement to insert
	sprintf(sqlBuff,
		"INSERT INTO BargnUtil  (Scenario, Turn_t,Bargn_i, Act_i, Util) VALUES ('%s',?1, ?2, ?3, ?4)",
		scenName.c_str());

	const char* insStr = sqlBuff;
	sqlite3_stmt *insStmt;
	sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
	// start for the transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

	for (unsigned int i = 0; i < Util_mat_col; i++) {
		for (unsigned int j = 0; j < Util_mat_row; j++)
		{

			int rslt = 0;
			// Turn_t
			rslt = sqlite3_bind_int(insStmt, 1, t);
			assert(SQLITE_OK == rslt);
			//Bargn_i
			rslt = sqlite3_bind_int(insStmt, 2, Bargn_i+i);
			assert(SQLITE_OK == rslt);
			//Act_i
			rslt = sqlite3_bind_int(insStmt, 3, j);
			assert(SQLITE_OK == rslt);
			//Util
			rslt = sqlite3_bind_double(insStmt, 4, Util_mat(j, i));
			assert(SQLITE_OK == rslt);
			// finish  
			assert(SQLITE_OK == rslt);
			rslt = sqlite3_step(insStmt);
			assert(SQLITE_DONE == rslt);
			sqlite3_clear_bindings(insStmt);
			assert(SQLITE_DONE == rslt);
			rslt = sqlite3_reset(insStmt);
			assert(SQLITE_OK == rslt);
		}
	}

	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);


	delete sqlBuff;
	sqlBuff = nullptr;

	smpDB = db;
}

void Model::sqlBargainVote(unsigned int t, int Bargn_i, int Bargn_j, KBase::KMatrix Util_mat)
{
	// initiate the database
	sqlite3 * db = smpDB;

	// Error message in case
	char* zErrMsg = nullptr;
	auto sqlBuff = newChars(200);

	int Util_mat_row = Util_mat.numR();
	int Util_mat_col = Util_mat.numC();


	// prepare the sql statement to insert
	sprintf(sqlBuff,
		"INSERT INTO BargnVote  (Scenario, Turn_t,Bargn_i,  Bargn_j, Act_i, Vote) VALUES ('%s',?1, ?2, ?3, ?4,?5)",
		scenName.c_str());

	const char* insStr = sqlBuff;
	sqlite3_stmt *insStmt;
	sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
	// start for the transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);

	for (unsigned int i = 0; i < Util_mat_col; i++) {
		for (unsigned int j = 0; j < Util_mat_row; j++)
		{

			int rslt = 0;
			// Turn_t
			rslt = sqlite3_bind_int(insStmt, 1, t);
			assert(SQLITE_OK == rslt);
			//Bargn_i
			rslt = sqlite3_bind_int(insStmt, 2, Bargn_i + i);
			assert(SQLITE_OK == rslt);

			//Bargn_j
			rslt = sqlite3_bind_int(insStmt, 3, Bargn_i + i);
			assert(SQLITE_OK == rslt);

			//Act_i
			rslt = sqlite3_bind_int(insStmt, 4, j);
			assert(SQLITE_OK == rslt);
			//Util
			rslt = sqlite3_bind_double(insStmt, 5, Util_mat(j, i));
			assert(SQLITE_OK == rslt);
			// finish  
			assert(SQLITE_OK == rslt);
			rslt = sqlite3_step(insStmt);
			assert(SQLITE_DONE == rslt);
			sqlite3_clear_bindings(insStmt);
			assert(SQLITE_DONE == rslt);
			rslt = sqlite3_reset(insStmt);
			assert(SQLITE_OK == rslt);
		}
	}

	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);


	delete sqlBuff;
	sqlBuff = nullptr;

	smpDB = db;
}
// populates record for table PosProb for each step of
// module run
void Model::sqlPosProb(unsigned int t) {
    assert(nullptr != smpDB);
    assert(t < history.size());
    State* st = history[t];
    // check module for null
    assert(nullptr != st);
    // initiate the database
    sqlite3 * db = smpDB;
    smpDB = nullptr;
    // Error message in case
    char* zErrMsg = nullptr;
    auto sqlBuff = newChars(200);
    // prepare the sql statement to insert
    sprintf(sqlBuff,
            "INSERT INTO PosProb (Scenario, Turn_t, Est_h,Pos_i, Prob) VALUES ('%s', ?1, ?2, ?3, ?4)",
            scenName.c_str());
    const char* insStr = sqlBuff;
    sqlite3_stmt *insStmt;
    sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
    // start for the transaction
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    // collect the information from each estimator,actor
    for (unsigned int h = 0; h < numAct; h++) { // estimator is h
        // calculate the probablity with respect to each estimator
        auto pn = st->pDist(h);
        auto pdt = std::get<0>(pn); // note that these are unique positions
        assert( fabs(1 - sum(pdt)) < 1e-4);
        auto unq = std::get<1>(pn);
        // for each actor pupulate the probablity information
        for (unsigned int i = 0; i < numAct; i++) {
            int rslt = 0;
            // Extract the probabity for each actor
            double prob = st->posProb(i, unq, pdt);
            rslt = sqlite3_bind_int(insStmt, 1, t);
            assert(SQLITE_OK == rslt);
            rslt = sqlite3_bind_int(insStmt, 2, h);
            assert(SQLITE_OK == rslt);
            rslt = sqlite3_bind_int(insStmt, 3, i);
            assert(SQLITE_OK == rslt);
            rslt = sqlite3_bind_double(insStmt, 4, prob);
            assert(SQLITE_OK == rslt);
            rslt = sqlite3_step(insStmt);
            assert(SQLITE_DONE == rslt);
            sqlite3_clear_bindings(insStmt);
            assert(SQLITE_DONE == rslt);
            rslt = sqlite3_reset(insStmt);
            assert(SQLITE_OK == rslt);
        }
    }
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
    printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);

    delete sqlBuff;
    sqlBuff = nullptr;

    smpDB = db; // give it the new pointer

    return;
}
// populates record for table PosProb for each step of
// module run
void Model::sqlPosVote(unsigned int t) {
    assert(nullptr != smpDB);
    assert(t < history.size());
    State* st = history[t];


    // check module for null
    assert(nullptr != st);
    // initiate the database
    sqlite3 * db = smpDB;
    smpDB = nullptr;
    // Error message in case
    char* zErrMsg = nullptr;
    auto sqlBuff = newChars(200);
    // prepare the sql statement to insert
    sprintf(sqlBuff,
            "INSERT INTO PosVote (Scenario, Turn_t, Est_h, Voter_k,Pos_i, Pos_j,Vote) VALUES ('%s', ?1, ?2, ?3, ?4,?5,?6)",
            scenName.c_str());
    const char* insStr = sqlBuff;
    sqlite3_stmt *insStmt;
    sqlite3_prepare_v2(db, insStr, strlen(insStr), &insStmt, NULL);
    // start for the transaction
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    auto vr = VotingRule::Proportional;
    // collect the information from each estimator

    for (unsigned int k = 0; k < numAct; k++) { // voter is k
        auto rd = st->model->actrs[k];
        for (unsigned int i = 0; i < numAct; i++) {
            for (unsigned int j = 0; j < numAct; j++) {
                for (unsigned int h = 0; h < numAct; h++) { // estimator is h
                    if (((h == i) || (h == j)) && (i!=j))
                    {
                        auto vij = rd->vote(h, i, j, st);
                        int rslt = 0;
                        rslt = sqlite3_bind_int(insStmt, 1, t);
                        assert(SQLITE_OK == rslt);
                        rslt = sqlite3_bind_int(insStmt, 2, h);
                        assert(SQLITE_OK == rslt);
                        //voter_k
                        rslt = sqlite3_bind_int(insStmt, 3, k);
                        assert(SQLITE_OK == rslt);
                        // position i
                        rslt = sqlite3_bind_int(insStmt, 4, i);
                        assert(SQLITE_OK == rslt);
                        //position j
                        rslt = sqlite3_bind_int(insStmt, 5, j);
                        assert(SQLITE_OK == rslt);
                        // vote ?
                        rslt = sqlite3_bind_double(insStmt, 6, vij);
                        assert(SQLITE_OK == rslt);
                        rslt = sqlite3_step(insStmt);
                        assert(SQLITE_DONE == rslt);
                        sqlite3_clear_bindings(insStmt);
                        assert(SQLITE_DONE == rslt);
                        rslt = sqlite3_reset(insStmt);
                        assert(SQLITE_OK == rslt);
                    }
                }
            }
        }
    }
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
    printf("Stored SQL for turn %u of all estimators, actors, and positions \n", t);

    delete sqlBuff;
    sqlBuff = nullptr;

    smpDB = db; // give it the new pointer

    return;
}
} // end of namespace

// --------------------------------------------
// Copyright KAPSARC. Open source MIT License.
// --------------------------------------------
