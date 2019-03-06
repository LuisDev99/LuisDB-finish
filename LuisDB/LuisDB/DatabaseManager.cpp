#include "DatabaseManager.h"
#include <fstream>
#include <direct.h>
#include <string>
#include <list>
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>  // for high_resolution_clock

using namespace std;


DatabaseManager::DatabaseManager()
{
}


DatabaseManager::~DatabaseManager()
{
}

void DatabaseManager::createTable()
{
	vector<string> columnsType, columnsName;
	string databaseName, path, tableName;
	int tamanoDeTabla = 0;

	vector<string> databases = get_all_files_names_within_folder("Databases");

	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. Cree una base de dato para crear una tabla.\n";
		system("pause");
		return;
	}


	cout << "  ### CREANDO TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {
			if (databaseName == str)
				isValidDatabase = true;
		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);

	cout << "Ingrese el nombre de la tabla a crear: ";
	cin >> tableName;

	cout << " \n --- Ingresando Columnas --- ";

	int menu = 0;
	string nombreColumna, tipoColumna;
	int tamanoChar;
	int tipoDatoElegido = 0;

	do{

		cout << "Tipos de datos disponibles\n1. Int\n2. Double\n3. Char\nEliga un tipo de dato: ";
		cin >> tipoDatoElegido;

		switch (tipoDatoElegido){
		case 1:
			tipoColumna = "I";
			tamanoDeTabla += 4;
			break;

		case 2:
			tipoColumna = "D";
			tamanoDeTabla += 8;
			break;

		case 3:
			cout << "Ingrese el tamano del char (Maximo 4000): ";
			cin >> tamanoChar;

			if (tamanoChar <= 0 || tamanoChar > 4000){
				cout << "Tamano no es valido. Regresando\n";
				continue;
			}

			tipoColumna = "C" + to_string(tamanoChar);
			tamanoDeTabla += tamanoChar;
			break;

		default:
			cout << "Eliga un tipo de dato mostrado arriba! \n";
			continue;
			break;
		}

		cout << "Ingrese el nombre de la columna: ";
		cin >> nombreColumna;

		cout << "\nInformacion -> " << nombreColumna << " Tipo: " << tipoColumna << endl;

		columnsName.push_back(nombreColumna);
		columnsType.push_back(tipoColumna);

		cout << "Escriba 2 si quiere dejar de crear columnas. Ingrese numero: ";
		cin >> menu;
		cout << endl;

	} while (menu != 2);

	cout << " --- Imprimiendo la informacion de la tabla --- \n";

	cout << "Nombre de la tabla: " << tableName << "\n  --- Campos --- \n\n\tNombre de la columna\tTipo de Dato\n";
	
	for (int i = 0; i < columnsName.size(); i++){
		cout << "\t\t" << columnsName[i] << "\t\t\t" << columnsType[i] << endl;
	}

	//Generate a string for the columns type and columns names with a delimeter. Example: 'I;c20;c10;D'
	string delimiterType = "", delimiterName= "";

	for (int i = 0; i < columnsName.size(); i++){
		delimiterName += columnsName[i];
		delimiterType += columnsType[i];

		if (i + 1 != columnsName.size()){
			delimiterName += ";";
			delimiterType += ";";
		}

	}

	char type[40];
	char name[60];
	char table[20];

	strcpy_s(type, delimiterType.c_str());
	strcpy_s(name, delimiterName.c_str());
	strcpy_s(table, tableName.c_str());

	string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

	fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
	DatabaseMetadata databaseMetaFromFile;

	file.seekg(0, ios::beg);
	file.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db

	Bloque bloque(databaseMetaFromFile.dataBlockSize, type, name, table, tamanoDeTabla);
	bloque.registersCount = 0;
	bloque.nextBlock = 0;

	//Multiply the index of the bit in the map by the block size and add the space of the metadata with the bit map space	
	int freeBlockIndexFinal = get_Free_Block_Index(file);

	file.seekp(freeBlockIndexFinal, ios::beg); // Move to the first free block to write the block which will be the new table

	file.write( reinterpret_cast<char *>(&bloque), sizeof(bloque));


	//Save the table name and the table position (in bytes) into the index file

	string fileNameIndex = "Databases//" + databaseName + "//" + databaseName + ".txt";
	fstream writer(fileNameIndex.c_str(), ios::app);

	writer << tableName << " " << freeBlockIndexFinal << '\n';
	writer.close();

	system("pause");
	return;


}

bool DatabaseManager::createDatabase()
{
	string databaseName, path;
	int datablockSize;
	int databaseSize;

	cout << "  ### CREANDO BASE DE DATO ### \n\n";

	cout << "Ingrese el nombre de la base de dato: ";
	cin >> databaseName;
	path = databaseName;

	cout << "Ingrese el tamaño (MB): ";
	cin >> databaseSize;

	cout << "Tamaño de bloques\n1. 512\n2. 1024\n3. 2048\n4. 4096\n5. 8192\nEliga un tamaño: ";
	cin >> datablockSize;

	switch (datablockSize) {

	case 1:
		datablockSize = 512;
		break;

	case 2:
		datablockSize = 1024;
		break;

	case 3:
		datablockSize = 2048;
		break;

	case 4:
		datablockSize = 4096;
		break;

	case 5:
		datablockSize = 8192;
		break;

	default:
		cout << "Ingreso una opcion invalida! El tamaño sera 4096 por defecto\n";
		datablockSize = 4096;
		break;

	}

	path = "Databases//" + databaseName;

	_mkdir(path.c_str()); //Create a directory for this database

	string pathToDatabaseDirectory = path + "//" + databaseName + ".dbb"; //Concatenate the extension

	fstream writer(pathToDatabaseDirectory.c_str(), ios::out | ios::binary);

	DatabaseMetadata dbMetadata(databaseSize, datablockSize);

	writer.seekp(0, ios::beg);

	/*First 12 bytes for the metadata of the db*/
	writer.write(reinterpret_cast<char *>(&dbMetadata), sizeof DatabaseMetadata);

	//Debuggy
	int currentPosition = writer.tellg();

	/*Next, the map of bytes which will help to find an empty block, write it with all values set to true*/
	writer.seekp(DataBaseMetadataSize, ios::beg); //Move the pointer after the database metadata size

	bool dummyBool = true;
	for (int i = 0; i < dbMetadata.cantidadBloques; i++) {
		writer.write(reinterpret_cast<char *>(&dummyBool), sizeof(bool));
	}


	//Debuggy
	currentPosition = writer.tellg();

	/*The rest, write the empty blocks after the map of bytes*/
	writer.seekg(DataBaseMetadataSize + dbMetadata.cantidadBloques, ios::beg);
	Bloque block(dbMetadata.dataBlockSize);

	int sizeofBlock = sizeof block;

	for (int i = 0; i < dbMetadata.cantidadBloques; i++) {
		writer.write(reinterpret_cast<char *>(&block), sizeofBlock);

		//Jump to the next start of the block by getting the current position of the file pointer plus the datablocksize minus the sizeof the block
		int nextBlockPosition = (int)writer.tellg() + (dbMetadata.dataBlockSize - sizeofBlock); //Move to the next block TODO: Instead of jumping write an empty char

		writer.seekg(nextBlockPosition, ios::beg);
	}

	currentPosition = writer.tellg();

	writer.close();
	cout << "Database Created\n ";
	system("pause");

	return true;
}

void DatabaseManager::dropDatabase()
{
	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");

	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### BORRANDO LA BASE DE DATO ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato a borrar(de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);

	string command = "cd Databases && rmdir /s " + databaseName;
	system(command.c_str());

	cout << "Base de dato borrada\n";
	system("cd .. && pause");
}

void DatabaseManager::dropTable()
{
	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");

	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### BORRANDO TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato donde esta la tabla (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);


	vector<string> tables = get_all_database_tables(databaseName);

	if (tables.empty()){

		cout << "No hay tabla dentro de esta base de dato. \n";
		system("pause");
		return;
	}

	cout << "  --- Imprimiendo las tablas de la base de dato --- \n";

	for (string str : tables)
		cout << "\t> " << str << '\n';

	bool isValidTable = false;
	string tableName;

	do{

		cout << "\nEscriba el nombre de la tabla a borrar (de la lista): ";
		cin >> tableName;

		for (string str : tables) {

			if (tableName == str)
				isValidTable = true;

		}

		if (isValidTable == false)
			cout << "Ingrese bien el nombre de la tabla (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidTable == false);

	int tablePosition = get_Table_Position(tableName, databaseName);
	
	//cout << "Position: " << tablePosition << endl;
	//system("pause");

	//Delete the table from database
	
	string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

	fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
	Bloque bloqueVacio;

	file.seekg(tablePosition, ios::beg);
	file.write(reinterpret_cast<char *>(&bloqueVacio), sizeof Bloque); // Write an empty block 
	file.close();

	//****** FALTA BORRAR ESTA TABLA EN EL ARCHIVO DE INDICES TAMBIEN ******

	string indexesFile = "Databases//" + databaseName + "//" + databaseName + ".txt";
	string tempFile    = "Databases//" + databaseName + "//temp.txt";

	string tableNameFromFile;
	int position;

	fstream reader(indexesFile.c_str());
	fstream writerr(tempFile.c_str(), ios::app);
	

	while (reader >> tableNameFromFile >> position){

		if (tableNameFromFile != tableName)
			writerr << tableNameFromFile << " " << position << '\n';

	}

	writerr.close();
	reader.close();

	const char *p = indexesFile.c_str();
	const char *tmp = tempFile.c_str();
	remove(indexesFile.c_str());
	rename(tmp, p);

	cout << "La tabla ha sido eliminada\n";
	system("pause");

}

void DatabaseManager::insert()
{
	int menu = 0;

	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");

	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### INSERTANDO DATOS A LA TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato donde esta la tabla (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);


	vector<string> tables = get_all_database_tables(databaseName);

	if (tables.empty()){

		cout << "No hay tabla dentro de esta base de dato. \n";
		system("pause");
		return;
	}

	cout << "  --- Imprimiendo las tablas de la base de dato --- \n";

	for (string str : tables)
		cout << "\t> " << str << '\n';

	bool isValidTable = false;
	string tableName;

	do{

		cout << "\nEscriba el nombre de la tabla a insertar registro (de la lista): ";
		cin >> tableName;

		for (string str : tables) {

			if (tableName == str)
				isValidTable = true;

		}

		if (isValidTable == false)
			cout << "Ingrese bien el nombre de la tabla (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidTable == false);


	do{
		auto start = std::chrono::high_resolution_clock::now();

		int tablePosition = get_Table_Position(tableName, databaseName);

		//========= JUMP TO THE TABLE POSITION READ METADATAFILE, PRINT THE TABLE COLUMNS AND THEN ASK FOR THE SIZE OF THE REGISTER ========

		string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

		//Read the database metadata (to get the datablock size of each block)
		fstream metatada(fileName.c_str(), ios::in | ios::out | ios::binary);
		DatabaseMetadata databaseMetaFromFile;

		metatada.seekg(0, ios::beg);
		metatada.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db
		metatada.close();
		//end

		fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);

 		file.seekg(tablePosition, ios::beg); //Place the pointer at the beggining of the block

		Bloque blockFromFile, parent; // Parent will be use to save the metadata of the original table and copy it to the blocks childs
		int size = sizeof blockFromFile;
		file.read(reinterpret_cast<char *> (&blockFromFile), sizeof blockFromFile); //Read the table in which we will insert the info
		parent = blockFromFile;

		parent.registersCount = 0;
		parent.hasSpace = 1;

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK THE SIZE OF THE BLOCK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		bool isNewBlock = false;
		int currentPosition = 0; //This will store the block's position that has space if theres is one

		while (blockFromFile.hasSpace == false){

			/* If we've reached a block that is full and has no pointer to its right, its means that we are at the end of the list.
			 * Soo what we do is get an empty block, write the same metadata that the parent had and insert the new register.
			 */
			if (blockFromFile.nextBlock == 0){

				int freeBlock = get_Free_Block_Index(file);

				blockFromFile.nextBlock = freeBlock; //Link the free block
				file.seekg(tablePosition, ios::beg);
				file.write(reinterpret_cast<char *> (&blockFromFile), sizeof blockFromFile); //Write the block to save its new next block soo that we can set parent equal to blockFromFile

				blockFromFile = parent;
				tablePosition = freeBlock;

				file.seekg(freeBlock, ios::beg);
				file.write(reinterpret_cast<char *>(&parent), sizeof parent);
				isNewBlock = true;
				cout << "A new block was created when inserting this data! \n";
				break;
			}

			currentPosition = blockFromFile.nextBlock;
			file.seekg(blockFromFile.nextBlock, ios::beg);
			file.read(reinterpret_cast<char *>(&blockFromFile), sizeof blockFromFile);
		}

		if (isNewBlock == false && currentPosition != 0){

			tablePosition = currentPosition;
		}
	
		// !!!!!!!!!!!!!!!!!!!!!!!!!!! INSERTING THE DATA INTO THE RIGHT POSITION OF THE BLOCK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		int lastRegisterPosition = tablePosition + DataBlockMetadataSize + (blockFromFile.registerSize * blockFromFile.registersCount);

		file.seekg(lastRegisterPosition, ios::beg); //Move the pointer at the end of the last register to write the new one

		string tableColumnsType = blockFromFile.columnsType;
		string tableColumnsName = blockFromFile.columnsName;
		string currentType;
		string sizer = "";
		string dataToInsert = "";
		string charInput;

		vector<string> types = split(tableColumnsType, ";");
		vector<string> name = split(tableColumnsName, ";");

		int integerInput;
		double doubleInput;

		stringstream buffer;

		//	?? ASK FOR THE CORRECT DATA

		for (int i = 0; i < types.size(); i++){

			if (types[i] == "I"){
				currentType = "(Integer)";
			} else if (types[i] == "D"){
				currentType = "(Double)";
			} else{
				currentType = "(Char";
				sizer = split(types[i], "C")[0]; //Get the size of the char by getting ride of the 'C' in the string
				currentType += "[" + sizer + "])";
			}

			cout << currentType << " " << name[i] << ": ";

			if (currentType == "(Integer)"){
				cin >> integerInput;
				buffer.write(reinterpret_cast<char *>(&integerInput), sizeof(int)); //Make this int binary
				dataToInsert += buffer.str();
			}
			else if (currentType == "(Double)"){
				cin >> doubleInput;
				buffer.write(reinterpret_cast<char *>(&doubleInput), sizeof(double)); //Make this double binary
				dataToInsert += buffer.str();
			}
			else {

				int charSize = std::atoi(sizer.c_str());
				cin.ignore();

				getline(cin, charInput); //Grab the whole text with spaces

				if (charInput.size() > charSize){
					cout << "La cantidad de palabras se exceden de lo acordado!";
					charInput = "H";
				}

				
				int remainingCharSpace = charSize - charInput.size();

				if (remainingCharSpace > 0){ //Fill the string with remaining char spaces

					charInput += ";";

					for (int i = charInput.size(); i < charSize; i++){ 
						charInput += "#";
					}
				}

				dataToInsert += charInput;
			}

		}

		file.write(dataToInsert.c_str(), blockFromFile.registerSize);

		//Increment registers count 
		file.seekg(tablePosition, ios::beg);
		blockFromFile.registersCount++;

		int spaceForAnotherRegister = blockFromFile.registerSize * (blockFromFile.registersCount + 1);
		int remainingRegisterSpace = databaseMetaFromFile.dataBlockSize - DataBlockMetadataSize;

		if ( spaceForAnotherRegister >= remainingRegisterSpace){ //Check ahead of time if the block will have space for another register

			blockFromFile.hasSpace = 0; //Set space to false
		}

		file.write(reinterpret_cast<char *>(&blockFromFile), sizeof blockFromFile);

		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;

		std::cout << "Tiempo de ejecucion: " << elapsed.count() << " s\n";

		cout << "Registro se ha insertado exitosamente! Tiempo de ejecucion : " << elapsed.count() <<" \nIngrese 1 si quiere insertar otra vez: ";
		cin >> menu;
	}while (menu == 1 );
	
}

void DatabaseManager::deleteTableData()
{
	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");


	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### BORRANDO DATOS DE UNA TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato donde esta la tabla (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);


	vector<string> tables = get_all_database_tables(databaseName);

	if (tables.empty()){

		cout << "No hay tabla dentro de esta base de dato. \n";
		system("pause");
		return;
	}

	cout << "  --- Imprimiendo las tablas de la base de dato --- \n";

	for (string str : tables)
		cout << "\t> " << str << '\n';

	bool isValidTable = false;
	string tableName;

	do{

		cout << "\nEscriba el nombre de la tabla a borrar registros (de la lista): ";
		cin >> tableName;

		for (string str : tables) {

			if (tableName == str)
				isValidTable = true;

		}

		if (isValidTable == false)
			cout << "Ingrese bien el nombre de la tabla (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidTable == false);

	int menu = 0;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DELETE BODY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	auto start = std::chrono::high_resolution_clock::now();

	int tablePosition = get_Table_Position(tableName, databaseName);

	string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

	//Read the database metadata (to get the datablock size of each block)
	fstream metatada(fileName.c_str(), ios::in | ios::out | ios::binary);
	DatabaseMetadata databaseMetaFromFile;

	metatada.seekg(0, ios::beg);
	metatada.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db
	metatada.close();
	//end

	fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
	Bloque blockFromFile;

	file.seekg(tablePosition, ios::beg); //Place the pointer at the beggining of the block
	file.read(reinterpret_cast<char *> (&blockFromFile), sizeof blockFromFile); //Read the table in which we will select the info

	if (blockFromFile.registersCount == 0){
		cout << "Esta tabla no tiene registros\n";
		system("pause");
		return;
	}

	string tableColumnsType = blockFromFile.columnsType;
	string tableColumnsName = blockFromFile.columnsName;
	vector<string> types = split(tableColumnsType, ";");
	vector<string> name = split(tableColumnsName, ";");

	//vector<bool> selectedColumnsToUpdate;
	vector<string> newInputs;
	vector<string> conditions;

	int seleccionarTodo = 0;

	
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CONDITION PART !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	char opcionCondicion;
	int cantidadCondiciones = 0, cantidadCondicionesCumplidas = 0;

	cout << "Desea actualizar con condicion? (s/n): ";
	cin >> opcionCondicion;

	if (opcionCondicion == 's'){

		for (int i = 0; i < types.size(); i++){


			cout << "Desea aplicar condicion a esta columna [" + name[i] + "]? (s/n): ";
			cin >> opcionCondicion;

			if (opcionCondicion == 's'){

				cantidadCondiciones++;
				string condicion;
				cout << "Donde " << name[i] << " sea igual a: ";
				cin >> condicion;
				conditions.push_back(condicion);

			}
			else {
				conditions.push_back("nocondition");
			}

		}

	}

	else{
		for (int i = 0; i < name.size(); i++)
			conditions.push_back("nocondition");
	}


	int registerSize = blockFromFile.registerSize;
	int remainingBlockSpace = databaseMetaFromFile.dataBlockSize - DataBlockMetadataSize;


	int integerInput;
	double doubleInput;
	string charInput, text;

	int newInt;
	double newDouble;
	string newChar;
	stringstream buffer;

	string intStr, doubleStr, charStr;

	vector<int> pointerBegginingBytes;

	bool alreadyAskForNewInputs = false;

	while (blockFromFile.nextBlock != 0 || blockFromFile.registersCount > 0) {

		int registersReadedCounter = 0;
		string input = "> ";

		while (registersReadedCounter < blockFromFile.registersCount) {

			input = "> ";
			pointerBegginingBytes.push_back((int)file.tellg());
			for (int i = 0; i < types.size(); i++){

				if (types[i] == "I"){

					file.read(reinterpret_cast<char *>(&integerInput), 4);
					intStr = to_string(integerInput);

					if (conditions[i] == intStr)
						cantidadCondicionesCumplidas++;

					buffer = stringstream();
					int deleteIntCode = 101010;
					buffer.write(reinterpret_cast<char *>(&deleteIntCode), sizeof(int));
					newInputs.push_back(buffer.str());


				} else if (types[i] == "D") {

					file.read(reinterpret_cast<char *>(&doubleInput), 8);
					doubleStr = to_string(doubleInput);

					if (conditions[i] == doubleStr)
						cantidadCondicionesCumplidas++;

					int deletedDoubleCode = 101010;
					buffer = stringstream();
					buffer.write(reinterpret_cast<char *>(&deletedDoubleCode), sizeof(double));
					newInputs.push_back(buffer.str());
							
				} else {

					string sizer = split(types[i], "C")[0]; //Get the size of the char by getting ride of the 'C' in the string
					int charSize = std::atoi(sizer.c_str());
					char *data = new char[charSize];
					file.read(data, charSize);
					charInput = data;

					text = split(charInput, "#")[0];
					text = split(text, ";")[0];

					charStr = text;

					if (conditions[i] == charStr)
						cantidadCondicionesCumplidas++;

					newChar = "@";
					int remainingCharSpace = charSize - newChar.size();

					if (remainingCharSpace > 0){ //Fill the string with remaining char spaces
						newChar += ";";

						for (int i = newChar.size(); i < charSize; i++){
							newChar += "#";
							}
						}

							
					}
					newInputs.push_back(newChar);
					input += text + "\t\t\t";
				
				}

			
			bool allConditionsAsserted = false;

			if (cantidadCondiciones == cantidadCondicionesCumplidas)
				allConditionsAsserted = true;

			if (allConditionsAsserted == true) {

				string newRegister = "";

				for (string buffer : newInputs)
					newRegister += buffer;

				int pointer = (int)file.tellg();
				int bytesMoveBack = pointer - blockFromFile.registerSize;

				file.seekg(bytesMoveBack, ios::beg); // Move the pointer back to the beginning of the register
				file.write(newRegister.c_str(), blockFromFile.registerSize);
				file.close();
				file.open(fileName.c_str(), ios::in | ios::out | ios::binary);
				file.seekg(bytesMoveBack + blockFromFile.registerSize, ios::beg);
			}

			registersReadedCounter++;
			cantidadCondicionesCumplidas = 0;
			alreadyAskForNewInputs = true;
		}

		if (blockFromFile.nextBlock == 0) {

			auto finish = std::chrono::high_resolution_clock::now();

			std::chrono::duration<double> elapsed = finish - start;

			std::cout << "Tiempo de ejecucion: " << elapsed.count() << " s\n";

			system("pause");
			return;
		}

		file.seekg(blockFromFile.nextBlock, ios::beg);
		file.read(reinterpret_cast<char *>(&blockFromFile), sizeof blockFromFile);
	}

	system("pause");
}

void DatabaseManager::select()
{

	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");

	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### SELECCIONANDO DATOS DE UNA TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato donde esta la tabla (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);


	vector<string> tables = get_all_database_tables(databaseName);

	if (tables.empty()){

		cout << "No hay tabla dentro de esta base de dato. \n";
		system("pause");
		return;
	}

	cout << "  --- Imprimiendo las tablas de la base de dato --- \n";

	for (string str : tables)
		cout << "\t> " << str << '\n';

	bool isValidTable = false;
	string tableName;

	do{

		cout << "\nEscriba el nombre de la tabla a seleccionar registros (de la lista): ";
		cin >> tableName;

		for (string str : tables) {

			if (tableName == str)
				isValidTable = true;

		}

		if (isValidTable == false)
			cout << "Ingrese bien el nombre de la tabla (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidTable == false);

	int menu = 0;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SELECT BODY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	auto start = std::chrono::high_resolution_clock::now();

	int tablePosition = get_Table_Position(tableName, databaseName);

	string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

	//Read the database metadata (to get the datablock size of each block)
	fstream metatada(fileName.c_str(), ios::in | ios::out | ios::binary);
	DatabaseMetadata databaseMetaFromFile;

	metatada.seekg(0, ios::beg);
	metatada.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db
	metatada.close();
	//end

	fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
	Bloque blockFromFile;

	file.seekg(tablePosition, ios::beg); //Place the pointer at the beggining of the block
	file.read(reinterpret_cast<char *> (&blockFromFile), sizeof blockFromFile); //Read the table in which we will select the info

	if (blockFromFile.registersCount == 0){
		cout << "Esta tabla no tiene registros\n";
		system("pause");
		return;
	}

	string tableColumnsType = blockFromFile.columnsType;
	string tableColumnsName = blockFromFile.columnsName;
	vector<string> types = split(tableColumnsType, ";");
	vector<string> name = split(tableColumnsName, ";");

	vector<bool> selectedColumns;
	vector<string> conditions;

	int seleccionarTodo = 0;

	cout << "Desea seleccionar todos los registros? \n\t1. Seleccionar Todo (*) \n\t2. Seleccionar por columnas\n\tIngrese una opcion: ";
	cin >> seleccionarTodo;

	if (seleccionarTodo <= 0 || seleccionarTodo > 2 ) {
		cout << "No ingreso una opcion valida, se seleccionaran todos los campos!\n";
		seleccionarTodo = 1;
	}

	if (seleccionarTodo == 1){

		for (int i = 0; i < name.size(); i++){
			selectedColumns.push_back(true);
		}

	} else {

		char opcion;
		for (int i = 0; i < name.size(); i++){

			cout << "Columna: " << name[i] << ". Desea seleccionar esta columna? (s/n): ";
			cin >> opcion;

			switch (opcion){

			case 's':
				selectedColumns.push_back(true);
				cout << "Columna seleccionada!\n";
				break;

			case 'n':
				selectedColumns.push_back(false);
				cout << "Columna no sera seleccionada\n";
				continue;
				break;

			default:
				cout << "No eligio una opcion valida (escriba 's' o 'n' ), se seleccionara la columna! \n";
				selectedColumns.push_back(true);
				cout << "Columna seleccionada por defalto!\n";
				break;
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CONDITION PART !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	char opcionCondicion;
	int cantidadCondiciones = 0, cantidadCondicionesCumplidas = 0;

	cout << "Desea seleccionar con condicion? (s/n): ";
	cin >> opcionCondicion;

	if (opcionCondicion == 's'){

		for (int i = 0; i < types.size(); i++){
			 
			if (selectedColumns[i] == true){

				cout << "Desea aplicar condicion a esta columna ["+name[i]+"]? (s/n): ";
				cin >> opcionCondicion;

				if (opcionCondicion == 's'){

					cantidadCondiciones++;
					string condicion;
					cout << "Donde " << name[i] << " sea igual a: ";
					cin >> condicion;
					conditions.push_back(condicion);

				} else {
					conditions.push_back("nocondition");
				}
				
			} else {
				conditions.push_back("nocondition");
			}
		}

	} else{
		for (int i = 0; i < name.size(); i++)
			conditions.push_back("nocondition");
	}


	int registerSize = blockFromFile.registerSize;
	int remainingBlockSpace = databaseMetaFromFile.dataBlockSize - DataBlockMetadataSize;
	

	string charInput, text;

	int integerInput;
	double doubleInput;
	bool isDeletedRow = false;

	//Print the header
	cout << "============= Impriendo Datos ====================\n\nColumnas: ";
	for (int i = 0; i < name.size(); i++){

		if (selectedColumns[i] == true){
			cout << name[i] << "\t\t\t";
		}
	}

	cout << endl;

	while (blockFromFile.nextBlock != 0 || blockFromFile.registersCount > 0) {

		int registersReadedCounter = 0;
		string input = "> ";

		while (registersReadedCounter < blockFromFile.registersCount) {

			input = "> ";

			for (int i = 0; i < types.size(); i++){

				if (types[i] == "I"){
					file.read(reinterpret_cast<char *>(&integerInput), sizeof(int));

					if (selectedColumns[i] == true){

						string intStr = to_string(integerInput);

						if (conditions[i] == intStr)
							cantidadCondicionesCumplidas++;

						if (intStr == "101010")
							isDeletedRow = true;

						input += to_string(integerInput) + "\t\t\t";
					}

				} else if (types[i] == "D") {
					file.read(reinterpret_cast<char *>(&doubleInput), sizeof(double));
					
					if (selectedColumns[i] == true){

						string doubleStr = to_string(doubleInput);

						if (conditions[i] == doubleStr)
							cantidadCondicionesCumplidas++;

						if (doubleStr == "101010")
							isDeletedRow = true;

						input += to_string(doubleInput) + "\t\t\t";
					}

				} else {
					string sizer = split(types[i], "C")[0]; //Get the size of the char by getting ride of the 'C' in the string
					int charSize = std::atoi(sizer.c_str());
					char *data = new char[charSize];
					file.read(data, charSize);
					charInput = data;

					text = split(charInput, "#")[0];
					text = split(text, ";")[0];

					if (selectedColumns[i] == true){

						string charStr = text;

						if (conditions[i] == charStr)
							cantidadCondicionesCumplidas++;

						if (charStr == "@")
							isDeletedRow = true;

						input += text + "\t\t\t";
					}
				}

			}

			
			bool allConditionsAsserted = false;

			if (cantidadCondiciones == cantidadCondicionesCumplidas)
				allConditionsAsserted = true;

			if (allConditionsAsserted == true && isDeletedRow == false)
				cout << input << endl;

			registersReadedCounter++;
			cantidadCondicionesCumplidas = 0;
			isDeletedRow = false;

		}

		if (blockFromFile.nextBlock == 0) {

			auto finish = std::chrono::high_resolution_clock::now();

			std::chrono::duration<double> elapsed = finish - start;

			std::cout << "Tiempo de ejecucion: " << elapsed.count() << " s\n";

			system("pause");
			return;
		}

		file.seekg(blockFromFile.nextBlock, ios::beg);
		file.read(reinterpret_cast<char *>(&blockFromFile), sizeof blockFromFile);
	}
	
	

	system("pause");
}

void DatabaseManager::update()
{
	string databaseName;
	vector<string> databases = get_all_files_names_within_folder("Databases");


	if (databases.empty()){

		cout << "No hay ninguna base de dato creada. \n";
		system("pause");
		return;
	}
	cout << "  ### ACTUALIZANDO DATOS DE UNA TABLA ### \n\n";
	cout << "  --- Imprimiendo las bases de datos --- \n";

	for (string str : databases)
		cout << "\t> " << str << '\n';

	bool isValidDatabase = false;

	do{

		cout << "\nEscriba el nombre de la base de dato donde esta la tabla (de la lista): ";
		cin >> databaseName;

		for (string str : databases) {

			if (databaseName == str)
				isValidDatabase = true;

		}

		if (isValidDatabase == false)
			cout << "Ingrese bien el nombre de la base de dato (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidDatabase == false);


	vector<string> tables = get_all_database_tables(databaseName);

	if (tables.empty()){

		cout << "No hay tabla dentro de esta base de dato. \n";
		system("pause");
		return;
	}

	cout << "  --- Imprimiendo las tablas de la base de dato --- \n";

	for (string str : tables)
		cout << "\t> " << str << '\n';

	bool isValidTable = false;
	string tableName;

	do{

		cout << "\nEscriba el nombre de la tabla a actualizar registros (de la lista): ";
		cin >> tableName;

		for (string str : tables) {

			if (tableName == str)
				isValidTable = true;

		}

		if (isValidTable == false)
			cout << "Ingrese bien el nombre de la tabla (de las que se muestran arriba) Intente de Nuevo\n";

	} while (isValidTable == false);

	int menu = 0;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! UPDATE BODY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	auto start = std::chrono::high_resolution_clock::now();

	int tablePosition = get_Table_Position(tableName, databaseName);

	string fileName = "Databases//" + databaseName + "//" + databaseName + ".dbb";

	//Read the database metadata (to get the datablock size of each block)
	fstream metatada(fileName.c_str(), ios::in | ios::out | ios::binary);
	DatabaseMetadata databaseMetaFromFile;

	metatada.seekg(0, ios::beg);
	metatada.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db
	metatada.close();
	//end

	fstream file(fileName.c_str(), ios::in | ios::out | ios::binary);
	Bloque blockFromFile;

	file.seekg(tablePosition, ios::beg); //Place the pointer at the beggining of the block
	file.read(reinterpret_cast<char *> (&blockFromFile), sizeof blockFromFile); //Read the table in which we will select the info

	if (blockFromFile.registersCount == 0){
		cout << "Esta tabla no tiene registros\n";
		system("pause");
		return;
	}

	string tableColumnsType = blockFromFile.columnsType;
	string tableColumnsName = blockFromFile.columnsName;
	vector<string> types = split(tableColumnsType, ";");
	vector<string> name = split(tableColumnsName, ";");

	vector<bool> selectedColumnsToUpdate;
	vector<string> newInputs;
	vector<string> conditions;

	int seleccionarTodo = 0;

	do {
		cout << "Desea actualizar todos los registros? \n\t1. Actualizar Todo\n\t2. Actualizar por columnas\n\tIngrese una opcion: ";
		cin >> seleccionarTodo;

		if (seleccionarTodo <= 0 || seleccionarTodo > 2) {
			cout << "No ingreso una opcion valida, intente de nuevo\n";
		}

	} while (seleccionarTodo <= 0 || seleccionarTodo > 2);


	if (seleccionarTodo == 1){

		for (int i = 0; i < name.size(); i++){
			selectedColumnsToUpdate.push_back(true);
		}
	}
	else {

		char opcion;
		for (int i = 0; i < name.size(); i++){

			cout << "Columna: " << name[i] << ". Desea seleccionar esta columna? (s/n): ";
			cin >> opcion;

			switch (opcion){

			case 's':
				selectedColumnsToUpdate.push_back(true);
				cout << "Columna seleccionada, sera actualizada!\n";
				break;

			case 'n':
				selectedColumnsToUpdate.push_back(false);
				cout << "Columna no sera seleccionada, entonces no sera updated\n";
				continue;
				break;

			default:
				cout << "No eligio una opcion valida (escriba 's' o 'n' ), se actualizara la columna! \n";
				selectedColumnsToUpdate.push_back(true);
				cout << "Columna seleccionada por defalto!\n";
				break;
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CONDITION PART !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	char opcionCondicion;
	int cantidadCondiciones = 0, cantidadCondicionesCumplidas = 0;

	cout << "Desea actualizar con condicion? (s/n): ";
	cin >> opcionCondicion;

	if (opcionCondicion == 's'){

		for (int i = 0; i < types.size(); i++){


			cout << "Desea aplicar condicion a esta columna [" + name[i] + "]? (s/n): ";
			cin >> opcionCondicion;

			if (opcionCondicion == 's'){

				cantidadCondiciones++;
				string condicion;
				cout << "Donde " << name[i] << " sea igual a: ";
				cin >> condicion;
				conditions.push_back(condicion);

			} else {
				conditions.push_back("nocondition");
			}

		}

	}

	else{
		for (int i = 0; i < name.size(); i++)
			conditions.push_back("nocondition");
	}


	int registerSize = blockFromFile.registerSize;
	int remainingBlockSpace = databaseMetaFromFile.dataBlockSize - DataBlockMetadataSize;


	int integerInput;
	double doubleInput;
	string charInput, text;

	int newInt;
	double newDouble;
	string newChar;
	stringstream buffer;

	string intStr, doubleStr, charStr;

	vector<int> pointerBegginingBytes;

	bool alreadyAskForNewInputs = false;

	while (blockFromFile.nextBlock != 0 || blockFromFile.registersCount > 0) {

		int registersReadedCounter = 0;
		string input = "> ";

		while (registersReadedCounter < blockFromFile.registersCount) {

			input = "> ";
			pointerBegginingBytes.push_back((int)file.tellg());
			for (int i = 0; i < types.size(); i++){

				if (types[i] == "I"){

					file.read(reinterpret_cast<char *>(&integerInput), 4);
					intStr = to_string(integerInput);

					if (conditions[i] == intStr)
						cantidadCondicionesCumplidas++;

					if (selectedColumnsToUpdate[i] == true){

						if (alreadyAskForNewInputs == false){
							cout << "Ingrese el nuevo dato a esta columna  -> [" + name[i] + "] -> Tipo " + types[i] + ": ";
							cin >> newInt;
							buffer.write(reinterpret_cast<char *>(&newInt), sizeof(int));
							newInputs.push_back(buffer.str());
						}

						input += to_string(integerInput) + "\t\t\t";

					} else { 

						buffer = stringstream();
						buffer.write(reinterpret_cast<char *>(&integerInput), sizeof(int));

						if (alreadyAskForNewInputs == false){
							newInputs.push_back(buffer.str());
						} else {
							newInputs[i] = buffer.str();
						}
					}

				} else if (types[i] == "D") {

					file.read(reinterpret_cast<char *>(&doubleInput), 8);
					doubleStr = to_string(doubleInput);

					if (conditions[i] == doubleStr)
						cantidadCondicionesCumplidas++;

					if (selectedColumnsToUpdate[i] == true){

						if (alreadyAskForNewInputs == false){
							cout << "Ingrese el nuevo dato a esta columna  -> [" + name[i] + "] -> Tipo " + types[i] + ": ";
							cin >> newDouble;
							buffer.write(reinterpret_cast<char *>(&newDouble), sizeof(double));
							newInputs.push_back(buffer.str());
						}

						input += to_string(doubleInput) + "\t\t\t";

					} else { 
						buffer = stringstream();
						buffer.write(reinterpret_cast<char *>(&doubleInput), sizeof(double));

						if (alreadyAskForNewInputs == false){
							newInputs.push_back(buffer.str());
						} else {
							newInputs[i] = buffer.str();
						}
					}

				} else {

					string sizer = split(types[i], "C")[0]; //Get the size of the char by getting ride of the 'C' in the string
					int charSize = std::atoi(sizer.c_str());
					char *data = new char[charSize];
					file.read(data, charSize);
					charInput = data;

					text = split(charInput, "#")[0];
					text = split(text, ";")[0];

					charStr = text;

					if (conditions[i] == charStr)
						cantidadCondicionesCumplidas++;

					if (selectedColumnsToUpdate[i] == true){

						//Update
						if (alreadyAskForNewInputs == false){
							cout << "Ingrese el nuevo dato a esta columna  -> [" + name[i] + "] -> Tipo " + types[i] + ": ";
							cin.ignore();
							getline(cin, newChar); //Grab the whole text with spaces

							if (newChar.size() > charSize){
								cout << "La cantidad de palabras se exceden de lo acordado!";
								newChar = "H";
							}

							int remainingCharSpace = charSize - newChar.size();

							if (remainingCharSpace > 0){ //Fill the string with remaining char spaces
								newChar += ";";

								for (int i = newChar.size(); i < charSize; i++){
									newChar += "#";
								}
							}

							newInputs.push_back(newChar);
						}

						input += text + "\t\t\t";
					} else { 
						
						if (alreadyAskForNewInputs == false){
							newInputs.push_back(charInput);
						} else {
							
							newInputs[i] = charInput;
						}
					}
				}

			}

			bool allConditionsAsserted = false;

			if (cantidadCondiciones == cantidadCondicionesCumplidas)
				allConditionsAsserted = true;

			if (allConditionsAsserted == true) {
				
				string newRegister = "";

				for (string buffer : newInputs)
					newRegister += buffer;

				int pointer = (int)file.tellg();
				int bytesMoveBack =  pointer - blockFromFile.registerSize;
				
				file.seekg( bytesMoveBack, ios::beg); // Move the pointer back to the beginning of the register
				file.write(newRegister.c_str(), blockFromFile.registerSize);
				file.close();
				file.open(fileName.c_str(), ios::in | ios::out | ios::binary);
				file.seekg(bytesMoveBack + blockFromFile.registerSize, ios::beg);
			}
				
			registersReadedCounter++;
			cantidadCondicionesCumplidas = 0;
			alreadyAskForNewInputs = true;
		}

		if (blockFromFile.nextBlock == 0) {

			auto finish = std::chrono::high_resolution_clock::now();

			std::chrono::duration<double> elapsed = finish - start;

			std::cout << "Tiempo de ejecucion: " << elapsed.count() << " s\n";

			system("pause");
			return;
		}

		file.seekg(blockFromFile.nextBlock, ios::beg);
		file.read(reinterpret_cast<char *>(&blockFromFile), sizeof blockFromFile);
	}

	system("pause");
}

vector<string> DatabaseManager::get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	string search_path = folder + "/*.dbb*";

	/* Explanation of command
	* Change to the database directory
	* && is used to execute two commands in one line
	* dir is to show all files */

	system("dir /n /b Databases > file_names.txt");
	std::fstream myStream("file_names.txt", std::fstream::in);
	string str;
	while (getline(myStream, str))
	{
		names.push_back(str);
	}
	return names;
}

std::vector<string> DatabaseManager::get_all_database_tables(std::string databaseName){

	vector<string> names;
	string file = "Databases//" + databaseName + "//" + databaseName + ".txt";
	int deadInt;
	string tableName;

	fstream reader(file.c_str());

	while (reader >> tableName >> deadInt){
		names.push_back(tableName);
	}

	return names;
}

int DatabaseManager::get_Table_Position(std::string _tableName, std::string _databaseName){

	string file = "Databases//" + _databaseName + "//" + _databaseName + ".txt";
	int position;
	string tableNameFromFile;

	fstream reader(file.c_str());

	while (reader >> tableNameFromFile >> position){
		
		if (tableNameFromFile == _tableName)
			return position;

	}

	return 0;
}

int DatabaseManager::get_Free_Block_Index(fstream &file){
	
	DatabaseMetadata databaseMetaFromFile;

	file.seekg(0, ios::beg);
	file.read(reinterpret_cast<char *>(&databaseMetaFromFile), sizeof(DatabaseMetadata)); //Read the metadata of the db

	bool isEmpty;
	int pointerToFreeBlock = 0;

	file.seekg(DataBaseMetadataSize, ios::beg);

	/* Find the first empty block by searching through the map pf bytes */
	for (int i = 0; i < databaseMetaFromFile.cantidadBloques; i++) {
		file.read(reinterpret_cast<char *>(&isEmpty), sizeof(bool));

		if (isEmpty == true){

			//Save the index of the free block 
			pointerToFreeBlock = i;

			//Move back one byte to overwrite the boolean
			file.seekg((int)file.tellg() - 1, ios::beg);

			//Tell that this block is not free anymore by overwriting the boolean
			isEmpty = false;
			file.write(reinterpret_cast<char *>(&isEmpty), sizeof(bool));
			break;
		}

	}

	if (isEmpty != false){
		cout << "Ya no hay bloques vacios disponibles. Base de dato llena! \n";
		return -1;
	}

	//Multiply the index of the bit in the map by the block size and add the space of the metadata with the bit map space	
	int freeBlockIndexFinal = (pointerToFreeBlock * databaseMetaFromFile.dataBlockSize) + DataBaseMetadataSize + databaseMetaFromFile.cantidadBloques;

	return freeBlockIndexFinal;
}

void DatabaseManager::readTable() {

	ifstream readerIndex("Databases//luiss//luiss.txt");
	string table;
	int position;

	readerIndex >> table >> position;

	ifstream reader("Databases//luiss//luiss.dbb", ios::in | ios::binary);
	Bloque dataTable;
	
	reader.seekg(position, ios::beg);

	int size = sizeof dataTable;

	reader.read(reinterpret_cast<char *> (&dataTable), sizeof dataTable);
	reader.close();
}

vector<string> DatabaseManager::split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

void DatabaseManager::writeTable(std::fstream &file, bool isNewTable){



}

//TEEST
/*fstream reader(pathToDatabaseDirectory.c_str(), ios::in | ios::binary);
DatabaseMetadata mFresh;

reader.seekg(0, ios::beg);

reader.read(reinterpret_cast<char *>(&mFresh), sizeof(DatabaseMetadata));

cout << mFresh.databaseSize << "-> " << mFresh.dataBlockSize << "--> " << mFresh.cantidadBloques;

//For the boolean vector

bool  erick;
fstream reader(pathToDatabaseDirectory.c_str(), ios::in | ios::binary);

reader.seekg(12, ios::beg);

for (int i = 0; i < dbMetadata.cantidadBloques; i++) {
reader.read(reinterpret_cast<char *>(&erick), sizeof(bool));
cout << erick;
}


cout << "Nombres: " << bloque.columnsName << " Tipos: " << bloque.columnsType << " Nombre Tabla: " << bloque.tableName;
cout << "\n Tamano Bloque " << bloque.registerSize;


system("pause");*/