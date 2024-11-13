igbCLITool()

NAME 

	igbCLITool - Adjust page numbers in iGigBook Bulk Upload file

SYNOPSIS

	igbCLITool	[-c <ComposerName>] [-d] [-f <offset>] [-p] <filename>

DESCRIPTION

	-c		Use supplied composer name for all entries

	-d		Create dummy entries if output less than 50 lines

	-f		Offset added to each page number

	-p		Page number not present in source lines.


	A complete Bulk Upload file will take the format specified in the Bulk Upload menu
	item of the iGigBook Configuration interface. It follows the format:
				
			"Tune Up",7,1,"Davis, Miles”
			“Tune Title”,<page number>,<number of pages>,”Composer”

	The tool can be invoked with the single argument of the path to the file if the file 
	is in the default format as specified above. All other fields when invoking the 
	tool are optional depending on the contents of the input file.

	For this tool to work the order of the songs in the bulk upload file must reflect 
	the order that the songs appear in the actual PDF. The iGigBook Bulk Upload feature 
	will allow the orders to differ but you better get the page numbers right.

	If the target book has pages before the first page of the first tune the number 
	of pre-music pages, this offset can be specified using the -f option. The 
	page numbers in the output file will be adjusted accordingly. The iGigBook 
	interface can also be used to set the offset.

	In the input file the contents of the page number field is ignored. This is the 
	value that is calculated by the tool. On input the values in this field are 
	dummy values. They can be anything, even missing. The following are all valid 
	input lines:

			"Tune Up",0,1,"Davis, Miles”
			"Tune Up",500,1,"Davis, Miles”
			"Tune Up",,1,"Davis, Miles”

	As a convenience, another option when creating the input file is to leave 
	the <page number> field out completely in each line. In this case use 
	the -p option and create each input file lines as follows:

			"Tune Up",1,"Davis, Miles”
			“Tune Title”,<number of pages>,”Composer”

	
	If the composer of all the songs in the book is the same the name can be 
	specified using the -c option. Be sure to quote the name with the double 
	quote character (“) if the name contains a space. In fact, to be safe always 
	quote the name. When using this option the Composer field in each song 
	entry must be omitted. It is an error for any line to contain Composer information.

	So, if Miles Davis published a book of tunes that he alone composed, with 
	the Bulk Upload text file called mdbook.txt the tool invocation would be 

			igbCLITool -c ”Miles Davis” mdbook.txt
	
	A sample line in the file with the Composer omitted would be

			“Milestones”,1,1
			“Tune Title”,<page number>,<number of pages>

	
	If the -p option was also used in the invocation, 

			igbCLITool -p —c ”Miles Davis” mdbook.txt

	the same line in the file would be 
	
			“Milestones”,1
			“Tune Title”,<number of pages>

	For the Composer name there is also a silent behavior not directly controlled 
	by an option. If the -c option is not specified but a line is missing the 
	Composer field the tool will use the iGigBook default Composer string. It 
	allows the Bulk Upload file to contain the Composer field when needed but 
	by inference will use the default value if it isn’t there. This is for 
	convenience as it relieves the need to enter the default field when the 
	composer’s name isn’t known.

	iGigBook requires a minimum of 50 entries in a Bulk Upload file. If the 
	source file does not contain 50 or more entries iGigBook will refuse the 
	upload. Using the -d will create dummy entries in the output file to satisfy 
	the minimum. The dummy entries will be tacked on to the end of the file. Each 
	entry has a unique song name that will alphabetize at the end of the song list. 
	Also the page number will specify that each dummy entry will appear at the 
	end of the book, after all the valid entries. The Composer field will contain 
	the iGigBook default composer “[Author]”. Once the bulk upload has succeeded it 
	is recommended that the dummy entries be deleted using the iGigBook 
	Configuration interface.

	The tool does some error checking and cleanup. For example, it removes blank 
	spaces that are not within quoted strings. It also checks for conflicts between 
	what is contained in each entry and what is expected based on iGigBook 
	requirements and command line options specified.

OUTPUT

	The original input file is left untouched. The tool creates two output files 
	and leaves them in the same folder as the input file. If the input filename is 			‘myIndexFile.txt’ the first resulting output file will be called 
	‘BlkUpld_myIndexFile.txt’. That is, the prefix ‘BlkUpld_’ will be 
	added to the input file name. This file is the result of the tool 
	processing and should be used as the Bulk Upload source file for iGigBook.

	If the tool is run and the file ‘BlkUpld_myIndexFile.txt’ already exists it 
	will not be overwritten. Instead a new output file will be named 
	‘BlkUpld1_myIndexFile.txt’. That is, the prefix ‘BlkUpld1_’ will be added 
	to the file name. This behavior continues, so that the output file is never 
	overwritten. Instead the numeral increases each time. So running the tool 
	a third time with the same input file will result in an output file called 
	‘BlkUpld2_myIndexFile.txt’, etc.

	The second output file created is called ‘info_myIndexFile.txt’. When the 
	tool is run progress information appears on the screen. Usually the amount of 
	information would be minimal. But if there are errors in the input file many 
	lines could appear. The ‘info_myIndexFile.txt’ contains a copy of everything 
	that appeared on the screen and can therefore be reviewed after the program 
	terminates. This is useful if there are a lot of errors in the input file. 
	This file is not overwritten and follows the same protocol as the other 
	output file. They are parallel files.

EXIT STATUS

	The tool returns 0 on success, non-zero on failure.

EXAMPLES

	Sample input file myIndexFile.txt

				“Killer Joe”,1,1,”Benny Golson”
				“Milestones”,1,2,”Miles Davis”
				“Red Clay”,1,2,”Freddie Hubbard”
				“Lush Life”,1,1,”Billy Strayhorn”
	
	Invocation and resulting output file BlkUpld_myIndexFile.txt

				igbCLITool myIndexFile.txt

				“Killer Joe”,1,1,”Benny Golson”
				“Milestones”,2,2,”Miles Davis”
				“Red Clay”,4,2,”Freddie Hubbard”
				“Lush Life”,6,1,”Billy Strayhorn”


	Sample input file myIndexFile.txt

				“Killer Joe”,1
				“Milestones”,2
				“Red Clay”,2
				“Lush Life”,1

	Invocation and resulting output file BlkUpld_myIndexFile.txt

				igbCLITool -p myIndexFile.txt
						or
				igbCLITool -p -c myIndexFile.txt

				“Killer Joe”,1,1,”[Author]”
				“Milestones”,2,2,”[Author]”
				“Red Clay”,4,2,”[Author]”
				“Lush Life”,6,1,”[Author]”

	

	Sample input file myIndexFile.txt

				“Milestones”,2
				“Tune Up”,1
				“Four”,1
				“So What”,1

	Invocation and resulting output file BlkUpld_myIndexFile.txt

				igbCLITool -p -c”Miles Davis” myIndexFile.txt

				“Milestones”,1,2,”Miles Davis”
				“Tune Up”,3,1,”Miles Davis”
				“Four”,4,1,”Miles Davis”
				“So What”,5,1,”Miles Davis”

	
	Sample input file myIndexFile.txt

				“Killer Joe”,,1,”Benny Golson”
				“Milestones”,,2
				“Red Clay”,,2
				“Lush Life”,,1,”Billy Strayhorn”
	
	Invocation and resulting output file BlkUpld_myIndexFile.txt

				igbCLITool myIndexFile.txt

				“Killer Joe”,1,1,”Benny Golson”
				“Milestones”,2,2,”[Author]”
				“Red Clay”,4,2,”[Author]”
				“Lush Life”,6,1,”Billy Strayhorn”


	Sample input file myIndexFile.txt

				“Killer Joe”,1,”Benny Golson”
				“Milestones”,2
				“Red Clay”,2
				“Lush Life”,1,”Billy Strayhorn”
	
	Invocation and resulting output file BlkUpld_myIndexFile.txt

				igbCLITool -p myIndexFile.txt

				“Killer Joe”,1,1,”Benny Golson”
				“Milestones”,2,2,”[Author]”
				“Red Clay”,4,2,”[Author]”
				“Lush Life”,6,1,”Billy Strayhorn”


	The last example might be the most useful version of a general input file 
	when creating a new index. Omitting the (otherwise ignored) page number and 
	only specifying the composer when known might make the Bulk Upload text file 
	creation easiest. Then simply save the resulting output file as the complete 
	source, possibly renaming it.

NOTES

	The -f offset option can be used if the size of the index is greater than 
	the 500 line limit enforced by the iGigBook Configuration interface. Partition 
	the input file into chunks less than 501 lines but greater than 49 lines 
	(because of the Bulk Upload minimum). Upload the chunks in order. Use the -f 
	option on each upload after the first setting its value to the last page 
	number of the last entry of the previous upload. 

	The -f offset option can also be used to do partial updates to an index, for 
	example, when adding a new entry to the middle of an existing index or new 
	entries scattered throughout the current index. In iGigBook if an index is 
	uploaded without first deleting the existing index then duplicates will 
	exist for all the repeated entries. So the entries that will be repeated 
	must first be deleted using the iGigBook Configuration interface. This 
	can be a ponderous task.

	Here is how to use the -f option instead of deleting the entire index and 
	then re-uploading a complete revised index. Using the iGigBook Configuration 
	interface delete all entries starting at the earliest part of the existing 
	index that will be changed. Create a Bulk Upload file that contains the first 
	new (or changed) entry and all that follow. Then use the -f offset option to 
	tell the tool the page number from which the revised index should start. This 
	offset should include any offset used in the original upload. If the new, 
	partial Bulk Upload file is too small then use the -d option as well. This 
	method trades off the number that have to be deleted using the clunky iGigBook
	Configuration interface with the ability to update the page numbers of changed 
	entries.
