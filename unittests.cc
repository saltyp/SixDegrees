/*
Test suite using boosttest.
skip Makefile and simply compile using eg `# g++ --std=c++17 -o test test-boost.cc` for header-only 
or `g++ -std=c++17 test-boost.cc -o testboost /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.a` for static build usage 
*/

#define BOOST_TEST_MODULE imdb Test 
#include <boost/test/unit_test.hpp> //using static build
// #include <boost/test/included/unit_test.hpp> //header-only too slow comppilation

#include "imdb.h" //unittests on imdb class and its methods
#include "imdb-utils.h"
#include <stdio.h>

#include "path.h" //unittests on path class

using namespace std;

BOOST_AUTO_TEST_CASE(test_basic) 
{
    BOOST_TEST(1);  // check boost unittest working 
    imdb db(determinePathToData()); // inlined in imdb-utils.h
    BOOST_TEST(db.good(), "IMDB path should be opened without incident");

    vector<film> credits;
    string player = "Tom Hanks";
    film movie = {"Splash", 1984};
    BOOST_TEST((db.getCredits(player, credits) && credits.size() != 0), "Tom Hanks should be in the database.");
    auto WasMovieInFilmography = [&](film movie) {return (std::find(credits.begin(), credits.end(), movie) != credits.end());};
    BOOST_TEST(WasMovieInFilmography(movie), player << " should have movie " << movie.title << " as part of their filmogrpahy.");
    film movie2 = {"Big", 1988};
    BOOST_TEST(WasMovieInFilmography(movie2), player << " should have movie " << movie.title << " as part of their filmogrpahy.");

    vector<string> cast;
    film movie_nonexistant = {"My Short Thin Turkish Sweet Sixteen", 1984};
    BOOST_TEST(!db.getCast(movie_nonexistant, cast), 
                movie_nonexistant.title << " should not be in the database. ");
    bool exists = db.getCast(movie, cast);
    auto IsActorInCast = [&](const string& player) {
                return (std::find(cast.begin(), cast.end(), player) != cast.end());};
    BOOST_TEST(exists, movie.title << " should be in the database. ");
    BOOST_TEST(( exists && (cast.size() != 0)), 
                "The movie " << movie.title << " should return a non-zero cast.");
    BOOST_TEST(IsActorInCast(player), player << " should be in the movie " << movie.title);

    string player2 = "Daryl Hannah";
    BOOST_TEST((db.getCredits(player2, credits) && credits.size() != 0), 
                player2<<" should also be in the database");

    // if movie doesn't exist then cast should be empty

    BOOST_TEST((!db.getCast(movie_nonexistant, cast) && (cast.size() == 0)), 
                movie.title << " should not be in the database & should clear cast vector. ");

    // Scarface 1 != Scarface 2
    film scarface1 = {"Scarface", 1932};
    film scarface2 = {"Scarface", 1983};
    string paco = "Al Pacino";
    exists = db.getCast(scarface1, cast);
    BOOST_TEST(!IsActorInCast(paco), paco << " should not be in the movie " << movie.title << " (1932)");
    exists = db.getCast(scarface2, cast);
    BOOST_TEST(IsActorInCast(paco), paco << " should be in the movie " << movie.title << " (1983)");

    // Unittests for path class: methods getLength, addConnection, getLastPlayer, undoConnection
    path p(player);
    BOOST_TEST(p.getLength() == 0, "Path should be empty at beginning");
    p.addConnection(movie, player2);
    BOOST_TEST(p.getLength() == 1, "Path should have 1 connection now");
    BOOST_TEST(p.getLastPlayer() == player2, "getLastPlayer should return " << player2);
    BOOST_TEST(p.getLength() == 1, "Path should have 1 connection now");
    p.undoConnection();
    BOOST_TEST(p.getLength() == 0, "Path should be empty at beginning");
    // test cloning
    path p2(player);
    p2 = p;
    BOOST_TEST(&p != &p2, "Cloning should give a new object, not just another pointer to the same object");


    // Unittest 1 for shortest path
    string newt = "Carrie Henn";
    string vasquez = "Jenette Goldstein";
    path shortest_path = db.generateShortestPath(newt, vasquez);
    cout <<  endl <<"Shortest path: " << shortest_path << endl;
    path should_be_path = path(newt);
    film aliens = {"Aliens", 1986};
    should_be_path.addConnection(aliens, vasquez);
    BOOST_TEST(shortest_path.getLength() == should_be_path.getLength(), "Shortest path should be:" << should_be_path);
    
    path shortest_path2 = db.generateShortestPath(player, player2);
    cout <<  endl <<"Shortest path: " << shortest_path2 << endl;
    path should_be_path2 = path(player);
    film movie3 = {"Boffo! Tinseltown's Bombs and Blockbusters", 2006};
    should_be_path2.addConnection(movie3, player2);
    BOOST_TEST(shortest_path2.getLength() == should_be_path2.getLength(), "Shortest path should be:" << should_be_path2);

    string source = "Mary Tyler Moore";
    string tgt = "Red Buttons";
    path shortest_path3 = db.generateShortestPath(source, tgt);
    cout << endl << "Shortest path: " << shortest_path3 << endl;
    BOOST_TEST(shortest_path3.getLength() == 2, "Shortest path btw "<< source << " and " << tgt << "should be length 2.");

    source = "Jerry Cain";
    tgt = "Kevin Bacon";
    shortest_path = db.generateShortestPath(source, tgt);
    cout << endl << "Shortest path: " << shortest_path << endl;
    BOOST_TEST(shortest_path.getLength() == 3, "Shortest path btw "<< source << " and " << tgt << "should be length 3.");

    // this one takes a long time : 
    source = "Danzel Muzingo";
    tgt = "Liseli Mutti";
    shortest_path = db.generateShortestPath(source, tgt);
    cout << endl << "Shortest path: " << shortest_path << endl;
    BOOST_TEST(shortest_path.getLength() == 5, "Shortest path btw "<< source << " and " << tgt << "should be length 5.");
}