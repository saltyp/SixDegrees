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
    auto WasMovieInFilmography = [&]() {return (std::find(credits.begin(), credits.end(), movie) != credits.end());};
    BOOST_TEST(WasMovieInFilmography(), player << " should have movie " << movie.title << " as part of their filmogrpahy.");

    vector<string> cast;
    bool exists = db.getCast(movie, cast);
    auto IsActorInCast = [&]() {return (std::find(cast.begin(), cast.end(), player) != cast.end());};
    BOOST_TEST(( exists && (cast.size() != 0)), "The movie " << movie.title << " should be in the database");
    BOOST_TEST(IsActorInCast(), player << " should be in the movie " << movie.title);

    string player2 = "Daryl Hannah";
    BOOST_TEST((db.getCredits(player2, credits) && credits.size() != 0), player2<<" should also be in the database");

}