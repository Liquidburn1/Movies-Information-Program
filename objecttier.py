##
# Hassan Ali Qadir
#CS 341 PROJECT 1
#UIN: 678537123
#
# File: objecttier.py
#
# Builds Movie-related objects from data retrieved through
# the data tier.
#
# Original author:
#   Prof. Joe Hummel
#   U. of Illinois, Chicago
#   CS 341, Spring 2022
#   Project #02
#
import datatier


##################################################################
#
# Movie:
#
# Constructor(...)
# Properties:
#   Movie_ID: int
#   Title: string
#   Release_Year: string
#
class Movie:

    def __init__(self, id, title, date):
        self._Movie_ID = id
        self._Title = title
        self._Release_Year = date

    @property
    def Movie_ID(self):
        return self._Movie_ID

    @property
    def Title(self):
        return self._Title

    @property
    def Release_Year(self):
        return self._Release_Year


##################################################################
#
# MovieRating:
#
# Constructor(...)
# Properties:
#   Movie_ID: int
#   Title: string
#   Release_Year: string
#   Num_Reviews: int
#   Avg_Rating: float
#
class MovieRating:

    def __init__(self, id, title, r_y, n_r, a_r):
        self._Movie_ID = id
        self._Title = title
        self._Release_Year = r_y
        self._Num_Reviews = n_r
        self._Avg_Rating = a_r

    @property
    def Movie_ID(self):
        return self._Movie_ID

    @property
    def Title(self):
        return self._Title

    @property
    def Release_Year(self):
        return self._Release_Year

    @property
    def Num_Reviews(self):
        return self._Num_Reviews

    @property
    def Avg_Rating(self):
        return self._Avg_Rating


##################################################################
#
# MovieDetails:
#
# Constructor(...)
# Properties:
#   Movie_ID: int
#   Title: string
#   Release_Date: string, date only (no time)
#   Runtime: int (minutes)
#   Original_Language: string
#   Budget: int (USD)
#   Revenue: int (USD)
#   Num_Reviews: int
#   Avg_Rating: float
#   Tagline: string
#   Genres: list of string
#   Production_Companies: list of string
#
class MovieDetails:

    def __init__(self, id, title, r_d, runtime, o_l, b, revenue, n_r, a_r, tl,
                 g, p_c):
        self._Movie_ID = id
        self._Title = title
        self._Release_Date = r_d
        self._Runtime = runtime
        self._Original_Language = o_l
        self._Budget = b
        self._Revenue = revenue
        self._Num_Reviews = n_r
        self._Avg_Rating = a_r
        self._Tagline = tl
        self._Genres = g
        self._Production_Companies = p_c

    @property
    def Movie_ID(self):
        return self._Movie_ID

    @property
    def Title(self):
        return self._Title

    @property
    def Release_Date(self):
        return self._Release_Date

    @property
    def Runtime(self):
        return self._Runtime

    @property
    def Original_Language(self):
        return self._Original_Language

    @property
    def Budget(self):
        return self._Budget

    @property
    def Revenue(self):
        return self._Revenue

    @property
    def Release_Year(self):
        return self._Num_Reviews

    @property
    def Num_Reviews(self):
        return self._Num_Reviews

    @property
    def Avg_Rating(self):
        return self._Avg_Rating

    @property
    def Tagline(self):
        return self._Tagline

    @property
    def Genres(self):
        return self._Genres

    @property
    def Production_Companies(self):
        return self._Production_Companies


##################################################################
#
# num_movies:
#
# Returns: # of movies in the database; if an error returns -1
#
def num_movies(dbConn):
    sql = "Select count(Movie_Id) From Movies"
    row = datatier.select_one_row(dbConn, sql, [])
    if row == None:
        return -1
    else:
        return row[0]


##################################################################
#
# num_reviews:
#
# Returns: # of reviews in the database; if an error returns -1
#
def num_reviews(dbConn):
    sql = "Select count(Rating) From Ratings"
    row = datatier.select_one_row(dbConn, sql, [])
    if row == None:
        return -1
    else:
        return row[0]


##################################################################
#
# get_movies:
#
# gets and returns all movies whose name are "like"
# the pattern. Patterns are based on SQL, which allow
# the _ and % wildcards. Pass "%" to get all stations.
#
# Returns: list of movies in ascending order by name;
#          an empty list means the query did not retrieve
#          any data (or an internal error occurred, in
#          which case an error msg is already output).
#
def get_movies(dbConn, pattern):
    sql = "Select Movie_Id,Title,strftime('%Y',Release_Date) From Movies where title like ? order by Title asc"
    rows = datatier.select_n_rows(dbConn, sql, [pattern])
    ## Creating an empty list
    retrow = []
    ## Loop to go over the rows list and make objects of each movie as the Movie object
    for i in rows:
        retrow.append(Movie(i[0], i[1], i[2]))
    return retrow


##################################################################
#
# get_movie_details:
#
# gets and returns details about the given movie; you pass
# the movie id, function returns a MovieDetails object. Returns
# None if no movie was found with this id.
#
# Returns: if the search was successful, a MovieDetails obj
#          is returned. If the search did not find a matching
#          movie, None is returned; note that None is also
#          returned if an internal error occurred (in which
#          case an error msg is already output).
#
def get_movie_details(dbConn, movie_id):
    sql="Select Movies.Movie_ID, Title, Date(Release_Date), Runtime, Original_Language, Budget, Revenue, Count(Rating), Avg(Rating), Tagline From Movies Left join Ratings on Movies.Movie_ID=Ratings.Movie_ID Left join Movie_Taglines on Movies.Movie_ID=Movie_Taglines.Movie_ID where Movies.Movie_ID== ? "
    rows=datatier.select_one_row(dbConn,sql,[movie_id])
    
    #To check if the movie doesnt exist
    if(rows[0]==None):
        return None
    
    ##if it does exist we do this
    sql="Select Genre_Name From Genres join Movie_Genres on Genres.Genre_ID=Movie_Genres.Genre_ID join Movies on Movie_Genres.Movie_ID=Movies.Movie_ID where Movie_Genres.Movie_ID = ? order by Genre_Name asc"
    genres=[]
    for i in datatier.select_n_rows(dbConn,sql,[movie_id]):
        genres.append(i[0])
    
    sql="Select Company_Name From Companies join Movie_Production_Companies on Movie_Production_Companies.Company_ID = Companies.Company_ID join Movies on Movie_Production_Companies.Movie_ID = Movies.Movie_ID where Movies.Movie_ID= ? order by Company_Name asc"
    mps=[]
    for i in datatier.select_n_rows(dbConn,sql,[movie_id]):
        mps.append(i[0])

    #To check if the avg ratings and tagline is None to prevent an error since it will format None into two decimal places and will give an error. So to prevent that I did this 

    if rows[9]==None:
        tagline=""
    else:
        tagline=rows[9]
    
    if(rows[8] == None):
        avg=0
    else:
        avg=rows[8]
        
    return MovieDetails (rows[0], rows[1], rows[2], rows[3], rows[4], rows[5], rows[6], rows[7], avg, tagline,genres,mps)
    

##################################################################
#
# get_top_N_movies:
#
# gets and returns the top N movies based on their average
# rating, where each movie has at least the specified # of
# reviews. Example: pass (10, 100) to get the top 10 movies
# with at least 100 reviews.
#
# Returns: returns a list of 0 or more MovieRating objects;
#          the list could be empty if the min # of reviews
#          is too high. An empty list is also returned if
#          an internal error occurs (in which case an error
#          msg is already output).
#
def get_top_N_movies(dbConn, N, min_num_reviews):
    sql = "Select Ratings.Movie_ID,Title,strftime('%Y',Release_Date),Count(Rating) as x, Avg(Rating) as y From Movies join Ratings on Movies.Movie_ID=Ratings.Movie_ID group by Ratings.Movie_ID Having x >= ? order by y desc limit ?"
    rows = datatier.select_n_rows(dbConn, sql, [min_num_reviews, N])
    retrows=[]
    #puts the values in their correct object properties
    for i in rows:
        retrows.append(MovieRating(i[0],i[1],i[2],i[3],i[4]))
    return retrows


##################################################################
#
# add_review:
#
# Inserts the given review --- a rating value 0..10 --- into
# the database for the given movie. It is considered an error
# if the movie does not exist (see below), and the review is
# not inserted.
#
# Returns: 1 if the review was successfully added, returns
#          0 if not (e.g. if the movie does not exist, or if
#          an internal error occurred).
#
def add_review(dbConn, movie_id, rating):
    sql="Select Title From Movies where Movie_ID=?"
    
    #To check if the movie exists in the database
    row=datatier.select_one_row(dbConn,sql,[movie_id])
    if(row==()):
        return 0
    
    #if it does we insert the ratings into the table 
    sql="Insert Into Ratings(Movie_ID,Rating) values(?,?) "
    rowcount=datatier.perform_action(dbConn,sql,[movie_id,rating])
    if(rowcount==0 or rowcount==-1):
        return 0
    else:
        return 1

##################################################################
#
# set_tagline:
#
# Sets the tagline --- summary --- for the given movie. If
# the movie already has a tagline, it will be replaced by
# this new value. Passing a tagline of "" effectively
# deletes the existing tagline. It is considered an error
# if the movie does not exist (see below), and the tagline
# is not set.
#
# Returns: 1 if the tagline was successfully set, returns
#          0 if not (e.g. if the movie does not exist, or if
#          an internal error occurred).
#
def set_tagline(dbConn, movie_id, tagline):
    sql="Select Movies.Movie_ID,Tagline From Movies left join Movie_Taglines on Movies.Movie_ID = Movie_Taglines.Movie_ID where Movies.Movie_ID == ?"
    
    row=datatier.select_n_rows(dbConn,sql,[movie_id])

    #Checks if the the movie exists or not
    if(row==[]):
        return 0
    
    #if it does then it checks if the tagline exists already or not and also checks for internal errors 
    elif(row[0][1]==None):
        sql="Insert Into Movie_Taglines(Movie_ID,Tagline) values(?,?)"
        rowcount=datatier.perform_action(dbConn,sql,[movie_id,tagline])
        if(rowcount==0 or rowcount==-1):
            return 0
        else:
            return 1

    #if tagline already exists it just updates it
    else:
        sql="Update Movie_Taglines set Tagline = ? where Movie_ID ==?"
        rowcount=datatier.perform_action(dbConn,sql,[tagline,movie_id])
        if(rowcount==0 or rowcount==-1):
            return 0
        else:
            return 1