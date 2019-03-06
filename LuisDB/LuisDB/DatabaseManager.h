#pragma once

#include <string>
#include <vector>
#include <fstream>

#define MegaBytesInBytesSize 1048576
#define DataBlockSize 512
#define DataBlockMetadataSize 136
#define DataBaseMetadataSize 12


struct DatabaseMetadata {

	int databaseSize, cantidadBloques, dataBlockSize;

	DatabaseMetadata(int _databaseSize, int _dataBlockSize) {

		databaseSize = _databaseSize * MegaBytesInBytesSize; //Convert it to Bytes
		cantidadBloques = databaseSize / _dataBlockSize; // Calculate the amount of block
		dataBlockSize = _dataBlockSize;
		
	}

	DatabaseMetadata(){ }

};

struct Bloque {

	/* Primeros 105 bytes para la metadata del bloque (cada bloque actuara como tabla)
	* cuando un bloque se llene escribir la misma metadata al bloque siguiente ***/

	Bloque(int datablocksize, int _registerSize) {
		int remainingSpace = datablocksize - DataBlockMetadataSize;
		//Data = new char[remainingSpace];
		nextBlock = 0;
		registerSize = _registerSize;
		hasSpace = true;
	}

	Bloque(int datablocksize) {
		int remainingSpace = datablocksize - DataBlockMetadataSize;
		//Data = new char[remainingSpace];
		nextBlock = 0;
		registerSize = 0;
		hasSpace = true;
	}

	Bloque(int datablocksize, char _columnsType[40], char _columnsName[60], char _tableName[20], int _registerSize) {

		int remainingSpace = datablocksize - DataBlockMetadataSize;
		//Data = new char[remainingSpace];

		strcpy_s(columnsName, _columnsName);
		strcpy_s(columnsType, _columnsType);
		strcpy_s(tableName, _tableName);

		nextBlock = 0;
		registerSize = _registerSize;
		hasSpace = true;
	}

	Bloque(){}

	char columnsType[40]; // Example: 'I;c20;c10;D' Let I be integer,c for char and the number for size and D for double
	char columnsName[60]; // Example: 'Nombre;Edad;Salario'
	char tableName[20];

	int nextBlock;
	int registerSize;
	int registersCount;
	int hasSpace; //Manage this as a boolean 0 is false
};

struct ok{
	char columnsType[40]; // Example: 'I;c20;c10;D' Let I be integer,c for char and the number for size and D for double
	char columnsName[60]; // Example: 'Nombre;Edad;Salario'
	char tableName[20];

	int nextBlock;
	int registerSize;
	int registersCount;
	int hasSpace;
	int isEmpty;
};

class DatabaseManager
{

public:
	DatabaseManager();
	~DatabaseManager();

	std::vector<std::string> get_all_files_names_within_folder(std::string folder);
	std::vector<std::string> get_all_database_tables(std::string folder);
	std::vector<std::string> split(const std::string& str, const std::string& delim);

	int get_Table_Position(std::string tableName, std::string databaseName);
	int get_Free_Block_Index(std::fstream &file);

	void writeTable(std::fstream &file, bool isNewTable);

	void createTable();
	bool createDatabase();
	void dropDatabase();
	void dropTable();
	void insert(); //Think
	void deleteTableData(); //Think
	void select();
	void update();

	void readTable();

};


struct typeData{

	int dataInt;
	char *dataChar;
	double dataDouble;

	typeData(int _data){
		dataInt = _data;
	}

	typeData(double _data){
		dataDouble = _data;
	}
};




