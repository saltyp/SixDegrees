using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <algorithm>
#include <list>
#include <set>
#include <iostream> // for debugging/info
#include <cassert>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
  // actorFile : first 4 bytes stores number of actors, then next 4 bytes offset to actorRecord  0, then actorRecord 1, then actorRecord 2, ...
  bool was_found = false;
  int* p_actorFile_start = (int*)actorFile;
  int num_actors = *p_actorFile_start; // first record of the file
  int* p_actor_offsets_start = p_actorFile_start + 1; // start of the actor offsets
  
  // binary search for the actor name:
  auto cmp = [p_actorFile_start](const int offset, const string& actor_name_to_find) {
    char* an_actor = (char*)p_actorFile_start + offset;
    return (an_actor < actor_name_to_find);
  };
  int* p_found_actor_offset = std::lower_bound(p_actor_offsets_start, (p_actor_offsets_start+num_actors), player, cmp);
  // get record of the actor if found:
  if (p_found_actor_offset != (p_actor_offsets_start+num_actors)) {
    was_found = true;
    // go to the actor record by pointing required num bytes past beginning of file:
    char* p_actor_record_start = (char*)actorFile + *p_found_actor_offset; 
    string actor_name = p_actor_record_start; // get the actor name == player 
    // get actor name & number of movies : 
    int actor_name_byte_length = sizeof(char)*actor_name.length();
    int actor_nummovies_byte_offset =  (actor_name_byte_length + 1) % 2 == 0 ? (actor_name_byte_length + 1) : (actor_name_byte_length + 2);
    int num_actor_movies = *(short*)(p_actor_record_start + actor_nummovies_byte_offset);
    int byte_offset_to_movieFile_offsets = actor_nummovies_byte_offset + sizeof(short);
    byte_offset_to_movieFile_offsets = byte_offset_to_movieFile_offsets % 4 == 0 ? byte_offset_to_movieFile_offsets : byte_offset_to_movieFile_offsets + 2;
    int* p_movieFile_offsets = (int*)(p_actor_record_start + byte_offset_to_movieFile_offsets); // CHECK
    // Fill in the films vector, by using the offsets stored in the actor_record to reference the movieFile
    // whose records contain (i) c-string of movie name, (ii) single byte rep of (movie_year - 1900), (iii) extra null padding if previous byte-amount is odd, 
    // (iv) short rep of num_actors, (v) possible 2 additonaly bytes of zero padding, then array of 4-byte int offsets into actorFile for each actor in the movie
    for (int j = 0; j < num_actor_movies; j++) {
      int movie_offset = *(p_movieFile_offsets + j);
      char* p_movie_record_start = (char*)movieFile + movie_offset;
      string movie_name = p_movie_record_start;
      int movie_name_byte_length = sizeof(char)*movie_name.length();
      // 1 byte for stored (year-1900) to which is added 1900 to give year again :
      int movie_year = *(char*)(p_movie_record_start + movie_name_byte_length + 1)+1900;
      film movie = {movie_name, movie_year};
      films.push_back(movie);
    }
  }
  return was_found;
}


bool imdb::getCast(const film& movie, vector<string>& players) const { 
  // the movieFile is akin to actorFile : int number of movies, int array of offsets for records, then the records, 
  // each of which contains (i) c-string of movie name, (ii) single byte rep of (movie_year - 1900), (iii) extra null padding if previous byte-amount is odd, 
  // (iv) short rep of num_actors, (v) possible 2 additonaly bytes of zero padding, then array of 4-byte int offsets into actorFile for each actor in the movie
  int* p_movieFile_start = (int*)movieFile;
  int num_movies = *p_movieFile_start; // first record of the file
  int* p_movie_offsets_start = p_movieFile_start + 1; // start of the movie offsets
  
  // binary search for the movie:
  auto cmpMovie = [p_movieFile_start](const int offset, const film& movie_to_find) {
    char* a_movie_name = (char*)p_movieFile_start + offset;
    string a_movie_string = a_movie_name;
    int a_movie_year = *(char*)(a_movie_name + a_movie_string.length() + 1) + 1900;
    film a_movie = {a_movie_string, a_movie_year};
    return (a_movie < movie_to_find);
  };
  int* lb = std::lower_bound(p_movie_offsets_start, (p_movie_offsets_start+num_movies), movie, cmpMovie);
  // get record of the movie if found: !TODO : this picks up regardless if movie is found or not
  char* p_movie_record_start = (char*)movieFile + *lb; 
  string movie_name = p_movie_record_start; // get the actor name == player
  bool was_found = (lb != (p_movie_offsets_start+num_movies)) && (movie.title == movie_name);
  if (was_found) {
    // get actor name & number of movies : 
    int movie_name_byte_length = sizeof(char)*movie_name.length(); // not including the '\0' at the end
    int cast_size_byte_offset =  (movie_name_byte_length + 1 + 1) % 2 == 0 ? (movie_name_byte_length + 2) : (movie_name_byte_length + 3); // 1 for year, 1 for null padding
    int cast_size = *(short*)(p_movie_record_start + cast_size_byte_offset);
    int byte_offset_to_actorFile_offsets = cast_size_byte_offset + sizeof(short);
    byte_offset_to_actorFile_offsets = byte_offset_to_actorFile_offsets % 4 == 0 ? byte_offset_to_actorFile_offsets : byte_offset_to_actorFile_offsets + 2;
    int* p_actorFile_offsets = (int*)(p_movie_record_start + byte_offset_to_actorFile_offsets);
    for (int j = 0; j < cast_size; j++) {
      int actor_offset = *(p_actorFile_offsets + j);
      char* p_actor_record_start = (char*)actorFile + actor_offset;
      string actor_name = p_actor_record_start;
      players.push_back(actor_name);
    }
  }
  else {
    players.clear();
  }
  return was_found; 
}

path imdb::generateShortestPath(const string& source, const string& target) const 
{
  // initialize
  const int MAXDEPTH = 7;
  path a_path_(source); // a 'state' which is the sum of previous actions necc to obtain optimal soln
  set<string> explored_actors {source};
  set<film> explored_films;
  // cout << "Shortest path Legend: '.' = film expanded, cast added; '*' = actor expanded  : " << endl;
  // iterative variables : 
  list<path> frontier_ {a_path_};
  list<path> next_;
  int depth = 0;
  vector<film> credits_;
  vector<string> cast_;
  string actor_;
  path another_path(source); // a different object
  
  while(!frontier_.empty()) {
    next_.clear();
    // for each path in the frontier
    while (!frontier_.empty()) {
      // CHOOSE & REMOVE from frontier
      a_path_ = frontier_.front(); frontier_.pop_front();
      // TEST
      if (a_path_.getLength() >= MAXDEPTH) {
        return path(source);
      }
      actor_ = a_path_.getLastPlayer();
      if (actor_ == target) return a_path_;
      // EXPAND : actor -[movies]-> other actors
      credits_.clear(); getCredits(actor_, credits_); cout << "*";
      for (film movie_:credits_) {
        if (explored_films.count(movie_)== 0) {
          cast_.clear(); getCast(movie_,cast_); cout << ".";
          for (string other_actor_ : cast_) {
            if (explored_actors.count(other_actor_)==0){
              // create another path to add to frontier
              another_path = a_path_; // copy by assignment 
              // assert(&another_path != &a_path_); // assert it's a deep copy
              another_path.addConnection(movie_, other_actor_);
              // if (other_actor_ == target) {
              //   return another_path; // SUCCESS !!!
              // }
              next_.push_back(another_path);
              explored_actors.insert(other_actor_);  // MEMOIZE
            }
          }
        explored_films.insert(movie_);  // MEMOIZE
        }
      }
    }
    frontier_ = next_;
    depth++;
  }
  // shouldn't reach here unless edge case (eg maxdepth not reached but no connecting paths)
  return path(source);
}


/*
BFS(s, Adj): #bfs to visit all the vertices ; “Gods algorithm"
	level = {s : 0}; // explored set (rep as a dict here)
	parent = {s:None};
	i = 1;  
	frontier = priority_queue(); # i is how many steps took to get there, frontier what you reach in (i-1); 
	frontier.append(s);
	# next is what is  reached in i steps; dict level is what you’ve seen before, parent is prev state/vertex
	while frontier: {
		next = [];
		for u in frontier:
			for v in Adj[u]: {
				if v not in level:
					level[v] = i; 
					parent[v] = u; 
					next.append(v) 
			}
		frontier = next; 
		i += 1; 
	}
*/







imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
