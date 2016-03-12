/**
 * Sokoban screen parser example for MACS 2
 * By Tom van Dijk <t.vandijk@utwente.nl
 *
 * Compile using: g++ -std=c++11 -Wall -Werror -O2 sokoban.cxx -o sokoban
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

enum class Field {EMPTY, WALL, MAN, BLOCK, GOAL, BLOCK_ON_GOAL, MAN_ON_GOAL};

// read a file, result is a vector of strings for each line, without trailing whitespace
static std::vector<std::string> readfile(std::istream& input) 
{
    std::vector<std::string> result;
    for (std::string line; std::getline(input, line);) {
        // get rid of trailing whitespace
        line = line.substr(0, line.find_last_not_of(" \t")+1);
        if (line.length() > 0) result.push_back(line);
    }
    return result;
}

// given a vector of strings, calculate the length of the longest string
static size_t max_line_length(std::vector<std::string>& lines)
{
    size_t max = 0;
    for (auto it = std::begin(lines); it != std::end(lines); it++) {
        if (it->length() > max) max = it->length();
    }
    return max;
}

// create a 'matrix' (vector of vectors) of size <rows> x <cols>, filled with <initial_value>
template <typename T>
static std::vector<std::vector<T>> create_matrix(const size_t rows, const size_t cols, const T initial_value)
{
    std::vector<std::vector<T>> result;
    for (size_t i = 0; i < rows; i++) {
        std::vector<T> row;
        row.assign(cols, initial_value);
        result.push_back(row);
    }
    return result;
}

// given the vector of strings of a screen input file, with <rows> rows and <cols> columns, return the screen in a matrix
static std::vector<std::vector<Field>> parse_screen(std::vector<std::string> lines, size_t rows, size_t cols)
{
    std::vector<std::vector<Field>> screen = create_matrix(rows, cols, Field::EMPTY);

    for (size_t i = 0; i < rows; i++) {
        std::string s = lines[i];
        std::vector<Field> &row = screen[i];

        for (size_t j = 0; j < s.length(); j++) {
            switch (s[j]) {
            case '#':
                row[j] = Field::WALL;
                break;
            case '@':
                row[j] = Field::MAN;
                break;
            case '.':
                row[j] = Field::GOAL;
                break;
            case ' ':
                row[j] = Field::EMPTY;
                break;
            case '$':
                row[j] = Field::BLOCK;
                break;
            case '*':
                row[j] = Field::BLOCK_ON_GOAL;
                break;
            case '+':
                row[j] = Field::MAN_ON_GOAL;
                break;
            default:
                std::cerr << "Invalid character '" << s[j] << "' in screen!" << std::endl;
                abort();
            }
        }
    }
    return screen;
}

// helper function for writing the screen from matrix to stream
static std::ostream& operator<<(std::ostream& stream, const Field field)
{
    switch (field) {
        case Field::WALL:
            stream << "#";
            break;
        case Field::MAN:
            stream << "@";
            break;
        case Field::GOAL:
            stream << ".";
            break;
        case Field::EMPTY:
            stream << " ";
            break;
        case Field::BLOCK:
            stream << "$";
            break;
        case Field::BLOCK_ON_GOAL:
            stream << "*";
            break;
        case Field::MAN_ON_GOAL:
            stream << "+";
            break;
    }
    return stream;
}

// helper function for writing a matrix to a stream
template<typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<std::vector<T>>& matrix)
{
    for (size_t i = 0; i < matrix.size(); i++) {
        std::vector<T> row = matrix[i];
        for (size_t j = 0; j < row.size(); j++) {
            stream << row[j];
        }
        stream << std::endl;
    }
    return stream;
}

static std::string getNUSMVField(const Field field)
{
    switch (field) {
        case Field::WALL:
            return "WALL";
			
		case Field::MAN:
            return "MAN";
			
        case Field::EMPTY:
            return "EMPTY";
           
        case Field::BLOCK:
            return "BLOCK";
			
		case Field::GOAL:
            return "GOAL";
			
		case Field::BLOCK_ON_GOAL:
            return "GOAL";
			
        case Field::MAN_ON_GOAL:
            return "GOAL";
        
		default :
			return "EMPTY";
        
    }
   // return "EMPTY";
}

static void WriteNUSMV(std::ofstream& stream, std::vector<std::vector<Field>> screen, size_t rows, size_t cols)
{
	size_t man_x;
	size_t man_y;
	
	std::vector<std::pair<size_t, size_t>> goals;

	stream << "MODULE main\n";
	stream << "VAR\n";
	stream << "\t--" << rows << "x" << cols << "\n";
	stream << "\tN_ROWS : " << (rows-1) << ".." << (rows-1) << ";\n";
	stream << "\tN_COLUMNS : " << (cols-1) << ".." << (cols-1) << ";\n";
	stream << "\n";
	stream << "\tman_x : 0..N_ROWS;\n";
	stream << "\tman_y : 0..N_COLUMNS;\n";
	stream << "\tman_move : {LEFT, RIGHT, UP, DOWN};\n";
	stream << "\n";
	stream << "\tmatrix : array 0..N_ROWS of array 0..N_COLUMNS of {WALL, EMPTY, BLOCK};\n";
	stream << "\n";

	
	
	stream << "ASSIGN\n";
	for (size_t i = 0; i < screen.size(); i++) {
        std::vector<Field> row = screen[i];
        for (size_t j = 0; j < row.size(); j++) {
			std::string value = getNUSMVField(row[j]);
			if(value == "MAN"){
				man_x = i;
				man_y = j;
				value = "EMPTY";				
			}
			
			if(value == "GOAL"){
				goals.push_back(std::make_pair(i, j));
				value = "EMPTY";
			}
			
			stream << "\tinit(matrix[" << i << "][" << j << "]) := " << value << ";\n";
        }
        stream << "\n";
    }
	
	stream << "\tinit(man_x) := " << man_x << ";\n";
	stream << "\tinit(man_y) := " << man_y << ";\n";
	stream << "\n";
	stream << "\tnext(man_x) :=\n";
	stream << "\tcase\n";
	stream << "\t\t(man_move = DOWN) & (man_x+1 <= N_ROWS) & (matrix[(man_x+1)][man_y] = EMPTY): man_x+1;\n";
	stream << "\t\t(man_move = DOWN) & (man_x+2 <= N_ROWS) & (matrix[(man_x+1)][man_y] = BLOCK & matrix[(man_x+2)][man_y] = EMPTY) : man_x+1;\n";
	stream << "\n";
	stream << "\t\t(man_move = UP) & (man_x+(-1) >= 0) & (matrix[(man_x+(-1))][man_y] = EMPTY): man_x+(-1);\n";
	stream << "\t\t(man_move = UP) & (man_x+(-2) >= 0) & (matrix[(man_x+(-1))][man_y] = BLOCK & matrix[(man_x+(-2))][man_y] = EMPTY) : man_x+(-1);\n";
	stream << "\n";
	stream << "\t\tTRUE : man_x;\n";
	stream << "\tesac;\n";
	stream << "\n";
	stream << "\tnext(man_y) :=\n";
	stream << "\tcase\n";
	stream << "\t\t(man_move = RIGHT) & (man_y+1 <= N_COLUMNS) & (matrix[man_x][(man_y+1)] = EMPTY)  : man_y+1;\n";
	stream << "\t\t(man_move = RIGHT) & (man_y+2 <= N_COLUMNS) & (matrix[man_x][(man_y+1)] = BLOCK & matrix[man_x][(man_y+2)] = EMPTY) : man_y+1;\n";
	stream << "\n";
	stream << "\t\t(man_move = LEFT) & (man_y+(-1) >= 0) & (matrix[man_x][(man_y+(-1))] = EMPTY)  : man_y+(-1);\n";
	stream << "\t\t(man_move = LEFT) & (man_y+(-2) >= 0) & (matrix[man_x][(man_y+(-1))] = BLOCK & matrix[man_x][(man_y+(-2))] = EMPTY) : man_y+(-1);\n";
	stream << "\n";
	stream << "\t\tTRUE : man_y;\n";
	stream << "\tesac;\n";
	stream << "\n";
	
	for (size_t i = 0; i < screen.size(); i++) {
        std::vector<Field> row = screen[i];
        for (size_t j = 0; j < row.size(); j++) {
			std::string value = getNUSMVField(row[j]);
			if(value == "WALL"){
				stream << "\tnext(matrix[" << i << "][" << j << "]) := matrix[" << i << "][" << j << "];\n";
				continue;
			}
		
		
			stream << "\tnext(matrix[" << i << "][" << j << "]) :=\n";
			stream << "\tcase\n";
			
			bool empty = false;
			
			if(j > 0 && j < row.size() - 1 && getNUSMVField(screen[i][j-1]) != "WALL" && getNUSMVField(screen[i][j+1]) != "WALL"){
				stream << "\t\t(man_move=RIGHT) & (man_x = " << i << ") & (man_y = " << (j-1) << ") & (matrix[" << i << "][" << j << "] = BLOCK) & (matrix[" << i << "][" << (j+1) << "] = EMPTY) : EMPTY;\n";
				stream << "\t\t(man_move=LEFT) & (man_x = " << i << ") & (man_y = " << (j+1) << ") & (matrix[" << i << "][" << j << "] = BLOCK) & (matrix[" << i << "][" << (j-1) << "] = EMPTY) : EMPTY;\n";
				
				empty = true;
			}
			
			if(i > 0 && i < screen.size() - 1 && getNUSMVField(screen[i-1][j]) != "WALL" && getNUSMVField(screen[i+1][j]) != "WALL"){
				stream << "\t\t(man_move=DOWN) & (man_x = " << (i-1) << ") & (man_y = " << j << ") & (matrix[" << i << "][" << j << "] = BLOCK) & (matrix[" << (i+1) << "][" << j << "] = EMPTY) : EMPTY;\n";
				stream << "\t\t(man_move=UP) & (man_x = " << (i+1) << ") & (man_y = " << j << ") & (matrix[" << i << "][" << j << "] = BLOCK) & (matrix[" << (i-1) << "][" << j << "] = EMPTY) : EMPTY;\n";
			
				empty = true;
			}
			
				if(empty)
				stream << "\n";
			
			if(j > 1){ 
				stream << "\t\t(man_move=RIGHT) & (man_x = " << i << ") & (man_y = " << (j-2) << ") & (matrix[" << i << "][" << (j-1) << "] = BLOCK) & (matrix[" << i << "][" << j << "] = EMPTY) : BLOCK;\n";
			}
			
			if(j < row.size() - 2){
				stream << "\t\t(man_move=LEFT) & (man_x = " << i << ") & (man_y = " << (j+2) << ") & (matrix[" << i << "][" << (j+1) << "] = BLOCK) & (matrix[" << i << "][" << j << "] = EMPTY) : BLOCK;\n";
			}
	   
			if(i > 1){
				stream << "\t\t(man_move=DOWN) & (man_x = " << (i-2) << ") & (man_y = " << j << ") & (matrix[" << (i-1) << "][" << j << "] = BLOCK) & (matrix[" << i << "][" << j << "] = EMPTY) : BLOCK;\n";
			}
			
			if(i < screen.size() - 2){
				stream << "\t\t(man_move=UP) & (man_x = " << (i+2) << ") & (man_y = " << j << ") & (matrix[" << (i+1) << "][" << j << "] = BLOCK) & (matrix[" << i << "][" << j << "] = EMPTY) : BLOCK;\n";
			}
			
			stream << "\t\tTRUE : matrix[" << i << "][" << j << "];\n";
			stream << "\tesac;\n";
			stream << "\n";
	   }
        stream << "\n";
    }	
	
	std::stringstream sspec1;
	std::stringstream sspec2;
		
	sspec1 << "CTLSPEC EF (";
	sspec2 << "CTLSPEC !EF (";
		
	for (size_t i = 0; i < goals.size(); i++) {
		auto pair = goals[i];
			
		sspec1 << "matrix[" << pair.first << "][" << pair.second << "] = BLOCK";
		sspec2 << "matrix[" << pair.first << "][" << pair.second << "] = BLOCK";
			
		if(i < goals.size()-1){
			sspec1 << " & ";
			sspec2 << " & ";
		}
	}
		
	stream << sspec1.str() << ");\n";
	stream << sspec2.str() << ");";	
}

int main(int argc, char* argv[]) {
    // Read file from stdin or argv[1] into vector of strings
    std::vector<std::string> lines;
    if (argc > 1) {
        std::ifstream ifs;
        ifs.open(argv[1], std::ifstream::in);
        lines = readfile(ifs);
        ifs.close();
    } else {
        lines = readfile(std::cin);
    }

    // Determine number of rows and columns
    auto rows = lines.size();
    if (rows == 0) {
        std::cerr << "No rows in screen!" << std::endl;
        abort();
    }
    auto cols = max_line_length(lines);
    if (cols == 0) {
        std::cerr << "No columns in screen!" << std::endl;
        abort();
    }

    std::cerr << "Screen with " << rows << " rows and " << cols << " columns" << std::endl;

    // Transform to matrix
    std::vector<std::vector<Field>> screen = parse_screen(lines, rows, cols);

    // Output screen
    std::cerr << screen;
	
	if(argc > 2){
		std::ofstream ofs;
		ofs.open (argv[2]);
		//ofs << "Writing this to a file.\n";
		WriteNUSMV(ofs,screen,rows,cols);
		ofs.close();
	}

    // Assignment 1b: Transform to nuSMV code
    // Assignment 2 : Use Sylvan to do model checking

    // You can use screen[row][column] to access fields
}
