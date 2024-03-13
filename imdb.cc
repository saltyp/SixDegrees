using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <algorithm>

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


bool imdb::getCast(const film& movie, vector<string>& players) const { return false; }

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
