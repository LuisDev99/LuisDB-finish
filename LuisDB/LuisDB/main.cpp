#include <iostream>
#include <string>
#include "DatabaseManager.h"
#include <sstream>
using namespace std;

//C:\Users\Familia\Documents\Visual Studio 2013\Projects\LuisDB\LuisDB\Databases


void main() {

	struct npm {
		char hello[20];
	};

	npm ok, ok2;
	strcpy_s(ok.hello, "Hola");

	fstream writer("double.txt", ios::in | ios::out | ios::binary);
	writer.write(reinterpret_cast<char *>(&ok), sizeof(npm));
	writer.seekg(0, ios::beg);
	writer.read(reinterpret_cast<char *>(&ok2), 20);
	writer.close();

	DatabaseManager database;
	int menu = 0;

	do {
		database.readTable();	
		
		system("cls");
		cout << " \t ### LuisDB Interactive DBCLI ###\n\n";
		cout << "1. Create Database\n2. Create Table\n3. Drop Database\n4. Drop Table\n";
		cout << "5. Select\n6. Update\n7. Delete\n8. Insert\n9. Salir\n";
		cout << "\n\t Ingrese la opcion $ ";

		cin >> menu;

		switch (menu) {

		case 1:
			database.createDatabase();
			break;

		case 2:
			database.createTable();
			break;

		case 3:
			database.dropDatabase();
			break;

		case 4:
			database.dropTable();
			break;

		case 5:
			database.select();
			break;

		case 6:
			database.update();
			break;

		case 7:
			database.deleteTableData();
			break;

		case 8:
			database.insert();
			break;
		}

	} while (menu != 9);

	system("pause");
}

//fstream writer("double.txt", ios::out | ios::binary);
//double rt = 33;
//int re = 2;
//double rt2;
//writer.write(reinterpret_cast<char *>(&rt), sizeof(double));
//writer.write(reinterpret_cast<char *>(&re), sizeof(int));
//writer.close();
//
//stringstream buffer;
//buffer.write(reinterpret_cast<char *>(&rt), 8);
//
//buffer.read(&buffer.str()[0], 8);
//cout << buffer.str();
//
//cout << reinterpret_cast<char *>(&rt);