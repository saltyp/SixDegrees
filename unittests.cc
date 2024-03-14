/*
Test suite using boosttest.
skip Makefile and simply compile using eg `# g++ --std=c++17 -o test test-boost.cc` for header-only 
or `g++ -std=c++17 test-boost.cc -o testboost /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.a` for static build usage 
*/

#define BOOST_TEST_MODULE imdb Test 
#include <boost/test/unit_test.hpp> //using static build
// #include <boost/test/included/unit_test.hpp> //header-only too slow comppilation

#include "imdb.h"
#include "imdb-utils.h"
#include <stdio.h>

#include "path.h"

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

    // Unittests for path class
    path p(player);
    BOOST_TEST(p.getLength() == 0, "Path should be empty at beginning");
    p.addConnection(movie, player2);
    BOOST_TEST(p.getLastPlayer() == player2, "getLastPlayer should return " << player2);
    BOOST_TEST(p.getLength() == 1, "Path should have 1 connection now");
    p.undoConnection();
    BOOST_TEST(p.getLength() == 0, "Path should be empty at beginning");



}