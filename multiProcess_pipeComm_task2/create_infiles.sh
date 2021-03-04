#!/bin/bash
GREEN="\033[0;32m"
RED="\033[31m"
RESET="\033[0m"

firstName_array=(Aaron Abigail Adam Adrian Adriana Adrienne Aimee Alan Albert Alec Alejandra Alejandro Alex Alexa Alexander Alexandra Alexandria Alexis Alfred Alice Alicia Alisha Alison Allen Allison Alvin Alyssa Amanda Amber Amy Ana Andre Andrea Andrew Angel Angela Angelica Angie Anita Ann Anna Anne Annette Anthony Antonio April Ariana Ariel Arthur Ashlee Ashley Audrey Austin Autumn Bailey Barbara Barry Becky Belinda Benjamin Bernard Beth Bethany Betty Beverly Bianca Bill Billy Blake Bob Bobby Bonnie Brad Bradley Brady Brandi Brandon Brandy Breanna Brenda Brendan Brent Brett Brian Briana Brianna Bridget Brittany Brittney Brooke Bruce Bryan Bryce Caitlin Caitlyn Caleb Calvin Cameron Candace Candice Carl Carla Carlos Carmen Carol Caroline Carolyn Carrie Casey Cassandra Cassidy Cassie Catherine Cathy Cesar Chad Charles Charlotte Chase Chelsea Chelsey Cheryl Chloe Chris Christian Christie Christina Christine Christopher Christy Cindy Claire Clarence Claudia Clayton Clifford Clinton Cody Cole Colin Colleen Collin Colton Connie Connor Corey Cory Courtney Craig Cristian Cristina Crystal Curtis Cynthia Daisy Dakota Dale Dalton Dan Dana Daniel Danielle Danny Darin Darius Darlene Darrell Darren Darryl Daryl Dave David Dawn Dean Deanna Debbie Deborah Debra Denise Dennis Derek Derrick Desiree Destiny Devin Devon Diana Diane Dillon Dominic Dominique Don Donald Donna Doris Dorothy Douglas Duane Dustin Dwayne Dylan Earl Ebony Eddie Edgar Eduardo Edward Edwin Eileen Elaine Elijah Elizabeth Ellen Emily Emma Eric Erica Erik Erika Erin Ernest Ethan Eugene Evan Evelyn Faith Felicia Fernando Frances Francis Francisco Frank Franklin Fred Frederick Gabriel Gabriela Gabriella Gabrielle Gail Garrett Gary Gavin Gene Geoffrey George Gerald Gilbert Gina Glen Glenda Glenn Gloria Gordon Grace Grant Greg Gregg Gregory Gwendolyn Hailey Haley Hannah Harold Harry Hayden Hayley Heather Hector Heidi Helen Henry Herbert Holly Howard Hunter Ian Isaac Isabel Isabella Isaiah Ivan Jack Jackie Jaclyn Jacob Jacqueline Jade Jaime Jake James Jamie Jane Janet Janice Jared Jasmin Jasmine Jason Javier Jay Jean Jeanette Jeanne Jeff Jeffery Jeffrey Jenna Jennifer Jenny Jeremiah Jeremy Jermaine Jerome Jerry Jesse Jessica Jesus Jill Jillian Jim Jimmy Jo Joan Joann Joanna Joanne Jocelyn Jodi Jody Joe Joel John Johnathan Johnny Jon Jonathan Jonathon Jordan Jorge Jose Joseph Joshua Joy Joyce Juan Judith Judy Julia Julian Julie Justin Kaitlin Kaitlyn Kara Karen Kari Karina Karl Karla Katelyn Katherine Kathleen Kathryn Kathy Katie Katrina Kayla Kaylee Keith Kelli Kellie Kelly Kelsey Kendra Kenneth Kent Kerri Kevin Kiara Kim Kimberly Kirk Kirsten Krista Kristen Kristi Kristie Kristin Kristina Kristine Kristopher Kristy Krystal Kurt Kyle Lacey Lance Larry Latasha Latoya Laura Lauren Laurie Lawrence Leah Lee Leon Leonard Leroy Leslie Linda Lindsay Lindsey Lisa Logan Loretta Lori Lorraine Louis Lucas Luis Luke Lynn Mackenzie Madeline Madison Makayla Malik Mallory Mandy Manuel Marc Marcia Marco Marcus Margaret Maria Mariah Marie Marilyn Mario Marisa Marissa Mark Martha Martin Marvin Mary Mason Mathew Matthew Maureen Maurice Max Maxwell Mckenzie Meagan Megan Meghan Melanie Melinda Melissa Melody Mercedes Meredith Mia Michael Michaela Micheal Michele Michelle Miguel Mikayla Mike Mindy Miranda Misty Mitchell Molly Monica Monique Morgan Nancy Natalie Natasha Nathan Nathaniel Neil Nicholas Nichole Nicolas Nicole Nina Noah Norma Olivia Omar Oscar Paige Pam Pamela Parker Patricia Patrick Patty Paul Paula Peggy Penny Peter Philip Phillip Phyllis Preston Priscilla Rachael Rachel Ralph Randall Randy Raven Ray Raymond Rebecca Rebekah Regina Renee Rhonda Ricardo Richard Rick Rickey Ricky Riley Rita Robert Roberta Roberto Robin Robyn Rodney Roger Ronald Ronnie Rose Ross Roy Ruben Russell Ruth Ryan Sabrina Sally Samantha Samuel Sandra Sandy Sara Sarah Savannah Scott Sean Selena Sergio Seth Shane Shannon Shari Sharon Shaun Shawn Shawna Sheena Sheila Shelby Shelia Shelley Shelly Sheri Sherri Sherry Shirley Sierra Sonia Sonya Sophia Spencer Stacey Stacie Stacy Stanley Stefanie Stephanie Stephen Steve Steven Sue Summer Susan Suzanne Sydney Sylvia Tamara Tami Tammie Tammy Tanner Tanya Tara Tasha Taylor Teresa Terrance Terrence Terri Terry Theresa Thomas Tiffany Tim Timothy Tina Todd Tom Tommy Toni Tony Tonya Tracey Traci Tracie Tracy Travis Trevor Tricia Tristan Troy Tyler Tyrone Valerie Vanessa Veronica Vicki Vickie Victor Victoria Vincent Virginia Walter Wanda Warren Wayne Wendy Wesley Whitney William Willie Wyatt Xavier Yesenia Yolanda Yvette Yvonne Zachary)
lastName_array=(Abbott Acevedo Acosta Adams Adkins Aguilar Aguirre Alexander Ali Allen Alvarado Alvarez Anderson Andrade Andrews Anthony Archer Arellano Arias Armstrong Arnold Arroyo Ashley Atkins Atkinson Austin Avery Avila Ayala Ayers Bailey Baird Baker Baldwin Ball Ballard Banks Barajas Barber Barker Barnes Barnett Barr Barrera Barrett Barron Barry Bartlett Barton Bass Bates Bauer Bautista Baxter Bean Beard Beasley Beck Becker Bell Beltran Bender Benitez Benjamin Bennett Benson Bentley Benton Berg Berger Bernard Berry Best Bishop Black Blackburn Blackwell Blair Blake Blanchard Blevins Bolton Bond Booker Boone Booth Bowen Bowers Bowman Boyd Boyer Boyle Bradford Bradley Bradshaw Brady Branch Brandt Braun Bray Brennan Brewer Bridges Briggs Bright Brock Brooks Brown Browning Bruce Bryan Bryant Buchanan Buck Buckley Bullock Burch Burgess Burke Burnett Burns Burton Bush Butler Byrd Cabrera Cain Calderon Caldwell Calhoun Callahan Camacho Cameron Campbell Campos Cannon Cantrell Cantu Cardenas Carey Carlson Carney Carpenter Carr Carrillo Carroll Carson Carter Case Casey Castaneda Castillo Castro Cervantes Chambers Chan Chandler Chaney Chang Chapman Charles Chase Chavez Chen Cherry Choi Christensen Christian Chung Church Cisneros Clark Clarke Clay Clayton Clements Cline Cobb Cochran Coffey Cohen Cole Coleman Collier Collins Colon Combs Compton Conley Conner Conrad Contreras Conway Cook Cooley Cooper Copeland Cordova Cortez Costa Cowan Cox Craig Crane Crawford Crosby Cross Cruz Cuevas Cummings Cunningham Curry Curtis Dalton Daniel Daniels Daugherty Davenport Davidson Davies Davila Davis Dawson Day Dean Decker Delacruz Deleon Delgado Dennis Diaz Dickerson Dickson Dillon Dixon Dodson Dominguez Donaldson Donovan Dorsey Douglas Downs Doyle Drake Dudley Duffy Duke Duncan Dunlap Dunn Duran Durham Dyer Eaton Edwards Elliott Ellis Ellison English Erickson Escobar Esparza Espinoza Estes Estrada Evans Ewing Farley Farmer Farrell Faulkner Ferguson Fernandez Ferrell Fields Figueroa Finley Fischer Fisher Fitzgerald Fitzpatrick Fleming Fletcher Flores Flowers Floyd Flynn Foley Forbes Ford Foster Fowler Fox Francis Franco Frank Franklin Frazier Frederick Freeman French Friedman Fritz Frost Fry Frye Fuentes Fuller Gaines Gallagher Gallegos Galloway Galvan Gamble Garcia Gardner Garner Garrett Garrison Garza Gates Gay Gentry George Gibbs Gibson Gilbert Giles Gill Gillespie Gilmore Glass Glenn Glover Golden Gomez Gonzales Gonzalez Good Goodman Goodwin Gordon Gould Graham Grant Graves Gray Green Greene Greer Gregory Griffin Griffith Grimes Gross Guerra Guerrero Gutierrez Guzman Haas Hahn Hale Haley Hall Hamilton Hammond Hampton Hancock Haney Hanna Hansen Hanson Hardin Harding Hardy Harmon Harper Harrell Harrington Harris Harrison Hart Hartman Harvey Hatfield Hawkins Hayden Hayes Haynes Hays Hebert Henderson Hendricks Hendrix Henry Hensley Henson Herman Hernandez Herrera Herring Hess Hester Hickman Hicks Higgins Hill Hines Hinton Ho Hobbs Hodge Hodges Hoffman Hogan Holden Holder Holland Holloway Holmes Holt Hood Hoover Hopkins Horn Horne Horton House Houston Howard Howe Howell Huang Hubbard Huber Hudson Huerta Huff Huffman Hughes Hull Humphrey Hunt Hunter Hurley Hurst Hutchinson Huynh Ibarra Ingram Irwin Jackson Jacobs Jacobson James Jefferson Jenkins Jennings Jensen Jimenez Johns Johnson Johnston Jones Jordan Joseph Joyce Juarez Kaiser Kane Kaufman Keith Keller Kelley Kelly Kemp Kennedy Kerr Key Khan Kidd Kim King Kirby Kirk Klein Kline Knapp Knight Knox Koch Kramer Krause Krueger Lam Lamb Lambert Landry Lane Lang Lara Larsen Larson Lawrence Lawson Le Leach Leblanc Lee Leon Leonard Lester Levine Levy Lewis Li Lin Lindsey Little Liu Livingston Lloyd Logan Long Lopez Love Lowe Lowery Lozano Lucas Lucero Luna Lynch Lynn Lyons Macdonald Macias Mack Madden Maddox Mahoney Maldonado Malone Mann Manning Marks Marquez Marsh Marshall Martin Martinez Mason Massey Mata Mathews Mathis Matthews Maxwell May Mayer Maynard Mayo Mays Mcbride Mccall Mccann Mccarthy Mccarty Mcclain Mcclure Mcconnell Mccormick Mccoy Mccullough Mcdaniel Mcdonald Mcdowell Mcfarland Mcgee Mcgrath Mcintosh Mcintyre Mckay Mckee Mckenzie Mckinney Mcknight Mclaughlin Mcmahon Mcmillan Mcneil Mcpherson Meadows Medina Mejia Melendez Melton Mendez Mendoza Mercado Mercer Merritt Meyer Meyers Meza Michael Middleton Miles Miller Mills Miranda Mitchell Molina Monroe Montes Montgomery Montoya Moody Moon Mooney Moore Mora Morales Moran Moreno Morgan Morris Morrison Morrow Morse Morton Moses Mosley Moss Moyer Mueller Mullen Mullins Munoz Murillo Murphy Murray Myers Nash Navarro Neal Nelson Newman Newton Nguyen Nichols Nicholson Nielsen Nixon Noble Nolan Norman Norris Norton Novak Nunez Obrien Ochoa Oconnell Oconnor Odom Odonnell Oliver Olsen Olson Oneal Oneill Orozco Orr Ortega Ortiz Osborn Osborne Owen Owens Pace Pacheco Padilla Page Palmer Park Parker Parks Parrish Parsons Patel Patrick Patterson Patton Paul Payne Pearson Peck Pena Pennington Perez Perkins Perry Peters Petersen Peterson Petty Pham Phelps Phillips Pierce Pineda Pittman Pitts Pollard Ponce Poole Pope Porter Potter Potts Powell Powers Pratt Preston Price Prince Proctor Pruitt Pugh Quinn Ramirez Ramos Ramsey Randall Randolph Rangel Rasmussen Ray Raymond Reed Reese Reeves Reid Reilly Reyes Reynolds Rhodes Rice Rich Richard Richards Richardson Richmond Riddle Riggs Riley Rios Ritter Rivas Rivera Rivers Roach Robbins Roberson Roberts Robertson Robinson Robles Rocha Rodgers Rodriguez Rogers Rojas Rollins Roman Romero Rosales Rosario Rose Ross Roth Rowe Rowland Roy Rubio Ruiz Rush Russell Russo Ryan Salas Salazar Salinas Sampson Sanchez Sanders Sandoval Sanford Santana Santiago Santos Saunders Savage Sawyer Schaefer Schmidt Schmitt Schneider Schroeder Schultz Schwartz Scott Sellers Serrano Sexton Shaffer Shah Shannon Sharp Shaw Shea Shelton Shepard Shepherd Sheppard Sherman Shields Short Silva Simmons Simon Simpson Sims Singh Singleton Skinner Small Smith Snow Snyder Solis Solomon Sosa Soto Sparks Spears Spence Spencer Stafford Stanley Stanton Stark Steele Stein Stephens Stephenson Stevens Stevenson Stewart Stokes Stone Stout Strickland Strong Stuart Sullivan Summers Sutton Swanson Sweeney Tanner Tapia Tate Taylor Terrell Terry Thomas Thompson Thornton Todd Torres Townsend Tran Travis Trevino Trujillo Tucker Turner Tyler Underwood Valdez Valencia Valentine Valenzuela Vance Vang Vargas Vasquez Vaughan Vaughn Vazquez Vega Velasquez Velazquez Velez Villa Villanueva Villarreal Villegas Vincent Wade Wagner Walker Wall Wallace Waller Walls Walsh Walter Walters Walton Wang Ward Ware Warner Warren Washington Waters Watkins Watson Watts Weaver Webb Weber Webster Weeks Weiss Welch Wells Werner West Wheeler Whitaker White Whitehead Whitney Wiggins Wilcox Wiley Wilkerson Wilkins Wilkinson Williams Williamson Willis Wilson Winters Wise Wolf Wolfe Wong Wood Woodard Woods Woodward Wright Wu Wyatt Yang Yates Yoder York Young Yu Zamora Zavala Zhang Zimmerman Zuniga)
firstName_size=${#firstName_array[@]}
lastName_size=${#lastName_array[@]}

function good() { echo -e "$GREEN [ PASS ]  $RESET"; }
function bad()  { echo -e "$RED [ FAIL ] $RESET"; exit 1; }
function fileCheck(){
	echo -n "  -Exists " 
	if [ -e $1 ]; 
		then good
	else
		bad
	fi
	
	echo -n "  -Regular file "
	if [ -f $1 ]; 
		then good
	else
		bad
	fi
	
	echo -n "  -Read permission "
	if [ -r $1 ]; 
		then good
	else
		bad
	fi

	echo -n "  -File not empty "
	if [ -s $1 ]; 
		then good
	else
		bad
	fi

}

function directoryCheck(){
	if [ -e $1 ]; 
		then echo "Directory $1 already exists"
		echo -n "- Want to replace it? [y/n] "
		read answer
		if [ $answer == "y" ];
			then rm -r $1
		else
			echo -n Script ended
			bad
		fi
	fi	
	mkdir $1
	echo -n -e "Directory $INPUT_DIRECTORY created"
	good
}

echo -e "\033[0;34m->$RESET Start";
echo -e "\033[0;34m->$RESET Script information:";
echo  "  - Number of input parameters = $#"
echo  "  - All arguments: $* "

numOfArguments=$#
DISEASES_FILE=$1
COUNTRIES_FILE=$2
INPUT_DIRECTORY=$3
NUM_FILES_PER_DIRECTORY=$4
NUM_RECORDS_PER_FILE=$5

echo -e "\033[0;34m->$RESET Check of arguments";
echo -n -e "\033[0;34mCHECK 1: $RESET"; 
if [ "$numOfArguments" \> 5 ]; 
	then echo -n "More arguments than wanted" 
	bad
elif [ "$numOfArguments" \< 5 ];
	then echo -n "Missing arguments"
	bad
elif [ "$numOfArguments" = 5 ];	
	then echo -n "Number of arguments is 5"
	good
fi

echo -n -e "\033[0;34mCHECK 2: $RESET";
if [ "$NUM_FILES_PER_DIRECTORY" -gt 0 ]; 
	then if [ "$NUM_RECORDS_PER_FILE" -gt 0 ]; 
			then echo -n "Valid number of files per directory and number of records per file"
			good
		else
			echo -n "NUM_RECORDS_PER_FILE can't be negative or 0" 
			bad
		fi
else
	echo -n "NUM_FILES_PER_DIRECTORY can't be negative or 0" 
	bad	 		
fi

echo -e "\033[0;34mCHECK 3: $RESET $DISEASES_FILE ";
fileCheck $DISEASES_FILE
echo -e "\033[0;34mCHECK 4: $RESET $COUNTRIES_FILE";
fileCheck $COUNTRIES_FILE
echo -e -n "\033[0;34mCHECK 7: $RESET";
directoryCheck $INPUT_DIRECTORY

dayBound=30
monthBound=12
yearUBound=2020
yearLBound=2000
id=100000

echo -e "\033[0;34m->$RESET Creating subdirectories of \033[0;35m$INPUT_DIRECTORY$RESET";
while read line; do
	echo -n "["
	subDirectoryPath="$INPUT_DIRECTORY/$line"
	mkdir $subDirectoryPath
	initD=1
	initM=1
	initY=2000

	for ((f=1; f<=$NUM_FILES_PER_DIRECTORY; f++)) do

		D=$((RANDOM%30+1))
		M=$((RANDOM%12+1))
		Y=$((RANDOM%20+2000))

		if [ $D -le 9 ]; 
			then D="0$D"
		fi

		if [ $M -le 9 ]; 
			then M="0$M"
		fi

		date="$D-$M-$Y"

		newfile="$INPUT_DIRECTORY/$line/$date"
		if [ -e $newfile ];
			then let "f-=1"
		else
			touch $newfile
		fi
	done


	fileArray=$(ls $subDirectoryPath | sort -t- -k3,3 -k2,2 -k1,1)
	# echo ${fileArray[@]}
	counter=0
	for file in ${fileArray[@]}; do

		filePath="$subDirectoryPath/$file"		
		if [ $NUM_FILES_PER_DIRECTORY -ge 20 ]; then 
			mod=$((counter%5))
			if [ $mod == 0 ];
				then echo -n -e "\033[0;34m#$RESET";
			fi 
		else
			echo -n -e "\033[0;34m#$RESET";
		fi
		let "counter+=1"


		for ((r=1; r<=$NUM_RECORDS_PER_FILE; r++)) do

			enterORexit_prob=$((RANDOM%3))
			tempfile="temp"
			if [ $enterORexit_prob == 0 ]; then 
				if [ ! -e $tempfile ]; 
					then if [ ! -s $tempfile ];
						then enterORexit_prob=1
					fi
				fi
			fi

			if [ $enterORexit_prob == 0 ]; then
				randomRecord=$(head -n 1 $tempfile)
				sed -i '1d' $tempfile
				echo $randomRecord >> $filePath
			else
				enterORexit="ENTER"
				age=$((RANDOM%120+1))
				lindex=$((RANDOM%lastName_size))
				findex=$((RANDOM%firstName_size))
				lastName=${lastName_array[$lindex]}
				firstName=${firstName_array[$findex]}
				diseaseName=$(shuf -n 1 $DISEASES_FILE)
				newRecord="$id $enterORexit $firstName $lastName $diseaseName $age"				
				echo $newRecord >> $filePath
				# let "id+=1"
				newRecord="$id EXIT $firstName $lastName $diseaseName $age"				
				echo $newRecord >> $tempfile
			fi
			let "id+=1"
		done
	done
	if [ -e $tempfile ]; 				
		then rm $tempfile
	fi
	echo -n -e "] $line \n" 
done < $COUNTRIES_FILE
echo -e "\033[0;34m<-$RESET End";

