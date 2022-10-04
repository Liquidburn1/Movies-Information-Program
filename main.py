#
# Hassan Ali Qadir
#CS 341 PROJECT 2
#UIN: 678537123
# This program uses python and sqllite 3 to use the implemented commands and get whatever information from the database MovieLens.db
#



### Functions ###

# -Main: Main function -line 209
#
# -genstats: This prints the stats like # of movies and # of reviews in total in the database -line 197
#
# -command_check: this checks which command was input and what to output because of that specific command -line 162

# -command1: this is the command to print out the movies -line 143

# -command2: this is the command to print out the movie details -line 102

# -command3: this is the command to print out the top N movie ratings as well as the number of reviews -line 78

# -command4: this is the command to add a rating for a movie -line 54

# -command5: this is the command to add a tagline for a movie in the database -line 34
import objecttier
import sqlite3





# Command 5: this is the command to add 
def command5(dbConn):
    print()
    print("tagline? ",end="")
    tag=input()
    print("movie id? ",end="")
    id=input()
    check=objecttier.set_tagline(dbConn,id,tag)
    if(check==1):
        print()
        print("Tagline successfully set")
        return
    else:
        print()
        print("No such movie...")





# Command 4: adds a rating for a movie and checks if the rating is valid and if the movie exits or not and then prints out if it was added
def command4(dbConn):
    print()
    print("Enter rating (0..10): ",end="")
    rating=input()
    if(int(rating)>10 or int(rating)<0):
        print("Invalid rating...")
        return
    print("Enter movie id: ",end="")
    id=input()
    check=objecttier.add_review(dbConn,id,int(rating))
    if(check==1):
        print()
        print("Review successfully inserted")
        return
    else:
        print()
        print("No such movie...")





# Command 3: Prints out the top N movies and their rating and number of reviews
def command3(dbConn):
    print("")
    print("N? ",end="")
    N=input()
    if(int(N)<1):
        print("Please enter a positive value for N...")
        return
    print("min number of reviews? ",end="")
    min= input()
    if(int(min)<=0):
        print("Please enter a positive value for min number of reviews...")
        return
    
    reviews=objecttier.get_top_N_movies(dbConn,int(N),int(min))
    print()
    for i in reviews:
        avgr=float(i.Avg_Rating)
        print(i.Movie_ID,":",i.Title,"("+i.Release_Year+"),","avg rating =",f"{(avgr):.2f}","("+str(i.Num_Reviews),"reviews)")
    
    
    
    
    
# Command 2: Gives the movie details after taking in the movie id
def command2(dbConn):
    print()
    print("Enter movie id: ",end='')
    id=input()
    details=objecttier.get_movie_details(dbConn,id)
    if(details==None):
        print()
        print("No such movie...")
    else:
        print()
        print(details.Movie_ID,":",details.Title)
        print("  Release date:",details.Release_Date)
        print("  Runtime:",details.Runtime,"(mins)")
        print("  Orig language:",details.Original_Language)
        print("  Budget: $"+f"{details.Budget:,}","(USD)")
        print("  Revenue: $"+f"{details._Revenue:,}","(USD)")
        print("  Num reviews:",details.Num_Reviews)
        print("  Avg rating:",f"{details.Avg_Rating:.2f}","(0..10)")
        #putting the commas in the list
        list1 = ", ".join(details.Genres) 
        #this if statement is for the comma that we have to after each company but not when there is no production company printed 3 lines longer but it works
        if(len(details.Genres)>0):
            print("  Genres:",list1+",")
        else:
            print("  Genres:")
        
        #putting the commas in the list
        list2= ", ".join(details.Production_Companies) #This is for priting the list without using a forloop
        
        #this if statement is for the comma that we have to after each company but not when there is no production company printed 3 lines longer but it works
        if(len(details.Production_Companies)>0):
            print("  Production companies:",list2+",")
        else:
            print("  Production companies:")
        print("  Tagline:",details.Tagline)
       
    



# Command 1: This prints out the movies after taking the string to search for
def command1(dbConn):
    print()
    print("Enter movie name (wildcards _ and % supported): ",end='')
    movie_name=input()
    movies=objecttier.get_movies(dbConn,movie_name)
    print()
    print("# of movies found:",len(movies))
    print()
    if(len(movies)>100):
        print("There are too many movies to display, please narrow your search and try again...")
    else:
        for i in movies:
            print(i.Movie_ID,":",i.Title,"("+i.Release_Year+")")





# Command check: checks which command was input and which command to input
def command_check(dbConn):
    print("Please enter a command (1-5, x to exit): ",end='')
    command=input()
    while(command!="x"):
        
        #command 1
        if(command=="1"):
            command1(dbConn)
        
        #command 2
        elif(command=="2"):
            command2(dbConn)
        
        #command 3
        elif(command=="3"):
            command3(dbConn)
       
        #command 4
        elif(command=="4"):
            command4(dbConn)
       
        #command 5
        elif(command=="5"):
            command5(dbConn)
        else:
            print("**Error, unknown command, try again...")
        print()
        print("Please enter a command (1-5, x to exit): ",end='')
        command=input()





# genstats: prints stats for movies
def genstats(dbConn):
    print("General stats:")
    num_movies=objecttier.num_movies(dbConn)
    print("  # of movies:",f"{num_movies:,}")
    num_reviews=objecttier.num_reviews(dbConn)
    print("  # of reviews:",f"{num_reviews:,}")
    print("")





# MAIN
print("** Welcome to the Movielens app **")
dbConn = sqlite3.connect('MovieLens.db')
genstats(dbConn)
command_check(dbConn)
dbConn.close()