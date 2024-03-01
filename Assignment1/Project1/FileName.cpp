#include <string>
#include <vector>
#include <cppconn/driver.h>
#include <cppconn/exception.h> 
#include <cppconn/statement.h> 
#include <iostream> 
#include <mysql_connection.h> 
#include <mysql_driver.h> 
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <set>
#include <ctime>  
#include <stdlib.h>

using namespace std;

int AVG_REC = 100;



int fine = 2000;

struct Date {
	int d, m, y;
};


const int monthDays[12]
= { 31, 28, 31, 30, 31, 30,
   31, 31, 30, 31, 30, 31 };


int countLeapYears(Date d)
{
	int years = d.y;

	
	if (d.m <= 2)
		years--;

	
	return years / 4
		- years / 100
		+ years / 400;
}


int getDifference(Date dt1, Date dt2)
{
		
	long int n1 = dt1.y * 365 + dt1.d;

	for (int i = 0; i < dt1.m - 1; i++)
		n1 += monthDays[i];

	n1 += countLeapYears(dt1);

	long int n2 = dt2.y * 365 + dt2.d;
	for (int i = 0; i < dt2.m - 1; i++)
		n2 += monthDays[i];
	n2 += countLeapYears(dt2);

	
	return (n2 - n1);
}





class User {

protected:
	string name;
	int id;
	string password;

public:
	void setName(const std::string& name) {
		this->name = name;
	}

	
	const std::string& getName() const {
		return name;
	}

	
	void setId(int id) {
		this->id = id;
	}

	
	int getId() const {
		return id;
	}

	
	void setPassword(const std::string& password) {
		this->password = password;
	}

	const std::string& getPassword() const {
		return password;
	}
};


class Manager : public User {

};
class Car {
	int id;
	string model;
	int condition;
	bool isRented;
	
	Date dueDate;
public:
	Car(int condition = 0, string model = "Aston Martin", bool isRented = false,Date dueDate={1,1,2024}, int id = 0)
	{
		this->condition = condition;
		this->model = model;
		this->isRented = isRented;
		this->dueDate = dueDate;
		this->id = id;
	}

	void request(Date dueDate)
	{
		this->isRented = true;
		this->dueDate = dueDate;
		
	}

	int ReturnCar(Date returnDate)
	{
		this->isRented = false;
		if (getDifference(this->dueDate, returnDate) > 0)
			return fine * getDifference(this->dueDate, returnDate);
		else return 0;
		
	}
	
	void setId(int id) {
		this->id = id;
	}

	
	int getId() const {
		return id;
	}

	
	void setModel(const std::string& model) {
		this->model = model;
	}


	const std::string& getModel() const {
		return model;
	}

	
	void setCondition(int condition) {
		this->condition = condition;
	}

	
	int getCondition() const {
		return condition;
	}

	
	void setIsRented(bool isRented) {
		this->isRented = isRented;
	}

	
	bool getIsRented() const {
		return isRented;
	}

	void setDueDate(Date date) {
		this->dueDate = date;
	}

	Date getDueDate(){
		return dueDate;
	}
	int retrieveFromDatabase(sql::Connection* conn, int carId) {
		int hi = 0;
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("SELECT * FROM Cars WHERE Id = ?");
			pstmt->setInt(1, carId);
			sql::ResultSet* res = pstmt->executeQuery();

			if (res->next()) {
				setId(res->getInt("Id"));
				setModel(res->getString("Model"));
				setCondition(res->getInt("CurrCondition"));
				setIsRented(res->getBoolean("IsRented"));
				setDueDate({ res->getInt("DueDay"),res->getInt("DueMonth"),res->getInt("DueYear") });
				hi = 1;
			}
			else {
				hi = 0;
			}

			delete res;
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
		return hi;
		
	}

	void updateToDatabase(sql::Connection* conn) {
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("UPDATE Cars SET Model = ?, CurrCondition = ?, IsRented = ?, DueDay = ?, DueMonth = ?, DueYear = ? WHERE Id = ?");
			pstmt->setString(1, getModel());
			pstmt->setInt(2, getCondition());
			pstmt->setBoolean(3, getIsRented());
			int a = getDueDate().d;
			int b = getDueDate().m;
			int c = getDueDate().y;
			pstmt->setInt(4, a);
			pstmt->setInt(5, b);
			pstmt->setInt(6, c);
			pstmt->setInt(7, getId());
			pstmt->executeUpdate();
			delete pstmt;
			std::cout << "Car with ID " << getId() << " updated successfully.\n";
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
	}
};

class Customer : public User {
private:
	std::vector<int> rented_cars;
	int fine_due;
	int customer_record;
	bool isEmployee;
public:
	Customer(std::string name = "", int id = 0, std::string password = "") :
		fine_due(0), customer_record(AVG_REC), isEmployee(false) {
		this->name = name;
		this->id = id;
		this->password = password;
	}

	
	void clearDue() {
		this->fine_due = 0;
	}

	void ReturnRequest(Car car,Date returnDate,int condition)
	{
		this->fine_due += car.ReturnCar(returnDate);
		auto it = find(this->rented_cars.begin(), this->rented_cars.end(),
			car.getId());
		if (car.ReturnCar(returnDate) == 0) this->customer_record += 10;
		else this->customer_record -= 5* car.ReturnCar(returnDate);

		if (car.getCondition() == condition) this->customer_record += 10;
		else this->customer_record -= 20 * (car.getCondition() - condition);

		if (customer_record < 0) customer_record = 0;
		if (it != this->rented_cars.end()) {
			this->rented_cars.erase(it);
		}

	}

	

	const std::vector<int>& getRentedCars() const {
		return rented_cars;
	}
	
	void setRentedCars(const std::vector<int>& cars) {
		rented_cars = cars;
	}
	
	int getFineDue() const {
		return fine_due;
	}
	
	void setFineDue(int fine) {
		fine_due = fine;
	}

	int getCustomerRecord() const {
		return customer_record;
	}

	void setCustomerRecord(int record) {
		customer_record = record;
	}
	
	bool getIsEmployee() const {
		return isEmployee;
	}

	void setIsEmployee(bool employee) {
		isEmployee = employee;
	}
	
	void insertIntoDatabase(sql::Connection* conn) {
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("INSERT INTO Customers (Name, Password, fine_due, customer_record, isEmployee) VALUES (?, ?, ?, ?, ?)");
			pstmt->setString(1, name);
			pstmt->setString(2, password);
			pstmt->setInt(3, fine_due);
			pstmt->setInt(4, customer_record);
			pstmt->setBoolean(5, isEmployee);
			pstmt->executeUpdate();
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
	}

	int retrieveFromDatabase(sql::Connection* conn, int customerId) {
		int hi = 0;
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("SELECT * FROM Customers WHERE Id = ?");
			pstmt->setInt(1, customerId);
			sql::ResultSet* res = pstmt->executeQuery();

			if (res->next()) {
				vector<int> cars;
				setName(res->getString("Name"));
				setId(res->getInt("Id"));
				setPassword(res->getString("Password"));
				setFineDue(res->getInt("fine_due"));
				setCustomerRecord(res->getInt("customer_record"));
				setIsEmployee(false);
				if (res->getInt("car1_id") != -1) cars.push_back(res->getInt("car1_id"));
				if (res->getInt("car2_id") != -1) cars.push_back(res->getInt("car2_id"));
				if (res->getInt("car3_id") != -1) cars.push_back(res->getInt("car3_id"));
				if (res->getInt("car4_id") != -1) cars.push_back(res->getInt("car4_id"));
				if (res->getInt("car5_id") != -1) cars.push_back(res->getInt("car5_id"));
				setRentedCars(cars);
				hi = 1;
			}
			else {
				hi = 0;

			}

			delete res;
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
		return hi;
	}

	int retrieveFromDatabaseEmp(sql::Connection* conn, int customerId) {
		int hi = 0;
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("SELECT * FROM Employees WHERE Id = ?");
			pstmt->setInt(1, customerId);
			sql::ResultSet* res = pstmt->executeQuery();

			if (res->next()) {
				vector<int> cars;
				setName(res->getString("Name"));
				setId(res->getInt("Id"));
				setPassword(res->getString("Password"));
				setFineDue(res->getInt("fine_due"));
				setCustomerRecord(res->getInt("customer_record"));
				setIsEmployee(true);
				if (res->getInt("car1_id") != -1) cars.push_back(res->getInt("car1_id"));
				if (res->getInt("car2_id") != -1) cars.push_back(res->getInt("car2_id"));
				if (res->getInt("car3_id") != -1) cars.push_back(res->getInt("car3_id"));
				if (res->getInt("car4_id") != -1) cars.push_back(res->getInt("car4_id"));
				if (res->getInt("car5_id") != -1) cars.push_back(res->getInt("car5_id"));
				setRentedCars(cars);
				hi = 1;
			}
			else {
				hi = 0;

			}

			delete res;
			delete pstmt;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
		return hi;
	}

	void updateToDatabase(sql::Connection* conn, const Customer& customer) {
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("UPDATE Customers SET Name = ?, Password = ?, fine_due = ?, customer_record = ?,car1_id = ?,car2_id = ?,car3_id = ?, car4_id = ?, car5_id = ? WHERE Id = ?");
			pstmt->setString(1, customer.getName());
			pstmt->setString(2, customer.getPassword());
			pstmt->setInt(3, customer.getFineDue());
			pstmt->setInt(4, customer.getCustomerRecord());
			int i = 0;
			for (i = 0; i < customer.getRentedCars().size(); i++)
				pstmt->setInt(5 + i, customer.getRentedCars()[i]);
			for (int j = i; j < 5; j++)
				pstmt->setInt(5 + j, -1);
			
			pstmt->setInt(10, customer.getId());
			pstmt->executeUpdate();
			delete pstmt;
			std::cout << "Customer with ID " << customer.getId() << " updated successfully." << std::endl;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
	}

	void updateToDatabaseEmp(sql::Connection* conn, const Customer& customer) {
		try {
			sql::PreparedStatement* pstmt = conn->prepareStatement("UPDATE Employees SET Name = ?, Password = ?, fine_due = ?, customer_record = ?,car1_id = ?,car2_id = ?,car3_id = ?, car4_id = ?, car5_id = ?  WHERE Id = ?");
			pstmt->setString(1, customer.getName());
			pstmt->setString(2, customer.getPassword());
			pstmt->setInt(3, customer.getFineDue());
			pstmt->setInt(4, customer.getCustomerRecord());
			int i = 0;
			for (i = 0; i < customer.getRentedCars().size(); i++)
				pstmt->setInt(5 + i, customer.getRentedCars()[i]);
			for (int j = i; j < 5; j++)
				pstmt->setInt(5 + j, -1);
			pstmt->setInt(10, customer.getId());
			pstmt->executeUpdate();
			delete pstmt;
			std::cout << "Employee with ID " << customer.getId() << " updated successfully." << std::endl;
		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
		}
	}
};


	


void addCustomers(sql::Connection* conn) {
	// Check if customers exist
	sql::ResultSet* res = conn->createStatement()->executeQuery("SELECT COUNT(*) AS count FROM Customers");
	res->next();
	int customerCount = res->getInt("count");
	delete res;

	if (customerCount == 0) {
		// Insert new customers
		for (int i = customerCount + 1; i <= 5; ++i) {
			std::string insertCustomerQuery = "INSERT INTO Customers (Name, Password) VALUES ('Customer" + std::to_string(i) + "', 'Password" + std::to_string(i) + "')";
			conn->createStatement()->execute(insertCustomerQuery);
		}
		
	}
	else {
		
	}
}

void addEmployees(sql::Connection* conn) {
	// Check if customers exist
	sql::ResultSet* res = conn->createStatement()->executeQuery("SELECT COUNT(*) AS count FROM Employees");
	res->next();
	int employeeCount = res->getInt("count");
	delete res;

	if (employeeCount == 0) {
		// Insert new customers
		for (int i = employeeCount + 1; i <= 5; ++i) {
			std::string insertEmployeeQuery = "INSERT INTO Employees (Name, Password) VALUES ('Employee" + std::to_string(i) + "', 'Password" + std::to_string(i) + "')";
			conn->createStatement()->execute(insertEmployeeQuery);
		}
		
	}
	
}

void addCars(sql::Connection* conn) {
	// Check if customers exist
	sql::ResultSet* res = conn->createStatement()->executeQuery("SELECT COUNT(*) AS count FROM Cars");
	res->next();
	int carCount = res->getInt("count");
	delete res;

	if (carCount == 0) {
		
		std::string carModels[5] = { "Toyota Camry", "Honda Civic", "Ford Mustang", "Chevrolet Corvette", "BMW 3 Series" };
		for (int i = carCount + 1; i <= 5; ++i) {
			std::string insertCarQuery = "INSERT INTO Cars (Model) VALUES ('" + carModels[i - 1] + "')";
			conn->createStatement()->execute(insertCarQuery);
			
		}
	}
	
}

bool isNum(string s)
{
	bool f = true;
	for (char i : s)
	{
		if (!isdigit(i))
		{
			f = false;
			break;
		}
	}
	return (f && s.length() < 10);
}

void inputCheck(string& in, set<string> vals, string prompt = "Wrong input.\nPlease enter again: ")
{
	getline(cin, in);
	while (vals.find(in) == vals.end())
	{   if(in!="")
		cout << prompt;
		getline(cin, in);
	}
}

void getAvailableCars(sql::Connection* conn)
{
	try {
		sql::PreparedStatement* pstmt = conn->prepareStatement("SELECT * FROM Cars WHERE IsRented = ?");
		pstmt->setBoolean(1, false);
		sql::ResultSet* res = pstmt->executeQuery();

		bool foundCars = false; // Flag to track if any cars are found

		while (res->next()) {
			foundCars = true; // Set flag to true if at least one car is found

			int id = res->getInt("Id");
			std::string model = res->getString("Model");
			int condition = res->getInt("CurrCondition");
			bool isRented = res->getBoolean("IsRented");

			// Do something with the car data (e.g., store in a vector, print, etc.)
			std::cout << "Car ID: " << id << ", Model: " << model << ", Condition: " << condition << std::endl;
		}

		delete res;
		delete pstmt;

		
		if (!foundCars) {
			std::cout << "No cars are available." << std::endl;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQL Error: " << e.what() << std::endl;
	}
}


void getCarsByIds(sql::Connection* conn, const std::vector<int>& carIds) {
	try {
		// Construct the WHERE clause dynamically using the provided car IDs
		std::string query = "SELECT Id, Model FROM Cars WHERE Id IN (";
		for (size_t i = 0; i < carIds.size(); ++i) {
			if (i > 0) {
				query += ",";
			}
			query += std::to_string(carIds[i]);
		}
		query += ")";

		
		sql::Statement* stmt = conn->createStatement();
		sql::ResultSet* res = stmt->executeQuery(query);

		while (res->next()) {
			int id = res->getInt("Id");
			std::string model = res->getString("Model");
			std::cout << "Car ID: " << id << ", Model: " << model << std::endl;
		}

		delete res;
		delete stmt;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQL Error: " << e.what() << std::endl;
	}
}


sql::ResultSet* getLastRow(sql::Connection* conn, const std::string& tableName) {
	try {
		sql::Statement* stmt = conn->createStatement();
		std::string query = "SELECT * FROM " + tableName + " ORDER BY Id DESC LIMIT 1";
		sql::ResultSet* res = stmt->executeQuery(query);
		delete stmt;
		return res;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQL Error: " << e.what() << std::endl;
		return nullptr;
	}
}

bool is_date_valid(int day, int month, int year) {
	if (year < 0 || month < 1 || month > 12 || day < 1) {
		return false;
	}

	if (month == 2) {
		if (year % 4 == 0 && year % 100 != 0) {
			if (day > 29) {
				return false;
			}
		}
		else {
			if (day > 28) {
				return false;
			}
		}
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11) {
		if (day > 30) {
			return false;
		}
	}
	else {
		if (day > 31) {
			return false;
		}
	}

	return true;
}

int deleteRow(sql::Connection* conn, const std::string& tableName, int id) {
	int hi = 0;
	try {
		
		sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM " + tableName + " WHERE Id = ?");
		checkStmt->setInt(1, id);
		sql::ResultSet* checkRes = checkStmt->executeQuery();

		if (checkRes->next()) {
			
			sql::PreparedStatement* pstmt = conn->prepareStatement("DELETE FROM " + tableName + " WHERE Id = ?");
			pstmt->setInt(1, id);
			pstmt->executeUpdate();
			delete pstmt;
			hi = 1;
		}
		else {
			hi = 0;
			
		}

		delete checkRes;
		delete checkStmt;
		return hi;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQL Error: " << e.what() << std::endl;
	}
}

int main()
{
	
		sql::mysql::MySQL_Driver* driver;
		sql::Connection* conn;

		try {
			driver = sql::mysql::get_mysql_driver_instance();
			conn = driver->connect("tcp://localhost:3306", "root", "iamhere@1309"); //Enter the username and password of MySQL

			conn->setSchema("mysql");
			
			sql::Statement* stmt1;
			stmt1= conn->createStatement();
			
			
			stmt1->execute("CREATE DATABASE IF NOT EXISTS test");
			conn->setSchema("test"); 

			sql::Statement* stmt;
			stmt = conn->createStatement();
			stmt->execute("CREATE TABLE IF NOT EXISTS Customers ("
				"Id INT AUTO_INCREMENT PRIMARY KEY,"
				"Name VARCHAR(255) NOT NULL,"
				"Password VARCHAR(255) NOT NULL,"
				"fine_due INT DEFAULT 0,"
				"customer_record INT DEFAULT 100,"
				"car1_id INT DEFAULT -1,"
				"car2_id INT DEFAULT -1,"
				"car3_id INT DEFAULT -1,"
				"car4_id INT DEFAULT -1,"
				"car5_id INT DEFAULT -1"
				")");
			delete stmt;

			stmt = conn->createStatement();
			stmt->execute("CREATE TABLE IF NOT EXISTS Employees ("
				"Id INT AUTO_INCREMENT PRIMARY KEY,"
				"Name VARCHAR(255) NOT NULL,"
				"Password VARCHAR(255) NOT NULL,"
				"fine_due INT DEFAULT 0,"
				"customer_record INT DEFAULT 100,"
				"car1_id INT DEFAULT -1,"
				"car2_id INT DEFAULT -1,"
				"car3_id INT DEFAULT -1,"
				"car4_id INT DEFAULT -1,"
				"car5_id INT DEFAULT -1"
				")");
			delete stmt;

			stmt = conn->createStatement();
			stmt->execute("CREATE TABLE IF NOT EXISTS Managers ("
				"Id INT AUTO_INCREMENT PRIMARY KEY,"
				"Name VARCHAR(255) NOT NULL,"
				"Password VARCHAR(255) NOT NULL"
				")");
			delete stmt;

			stmt = conn->createStatement();
			stmt->execute("CREATE TABLE IF NOT EXISTS Cars ("
				"Id INT AUTO_INCREMENT PRIMARY KEY,"
				"Model VARCHAR(255) NOT NULL,"
				"CurrCondition INT DEFAULT 10,"
				"IsRented BOOLEAN DEFAULT false,"
				"DueDay INT DEFAULT 1,"
				"DueMonth INT DEFAULT 1,"
				"DueYear INT DEFAULT 2024"
				")");
			delete stmt;

			addCustomers(conn);
			addEmployees(conn);
			addCars(conn);

			sql::ResultSet* res = conn->createStatement()->executeQuery("SELECT COUNT(*) AS count FROM Managers");
			res->next();
			int ManagerCount = res->getInt("count");
			delete res;
			if (ManagerCount == 0)
			{
				stmt = conn->createStatement();
				std::string insertManagerQuery = "INSERT INTO Managers (Name, Password) VALUES ('Overlord','lmao_ded')";
				stmt->execute(insertManagerQuery);
				std::cout << "Inserted Manager Successfully\n";
				delete stmt;
			}
			


		}
		catch (sql::SQLException& e) {
			std::cerr << "SQL Error: " << e.what() << std::endl;
			return 1;
		}
		while (true)
		{
			cout << "Welcome to My Car Rental System :)\n";
			cout << "Press 1 for Customer Login \n";
			cout << "Press 2 for Employee Login \n";
			cout << "Press 3 for Manager Login \n";
			cout << "Press 4 to end the program\n";
			cout << "Note: A new user can be registered by a manager only. To register, please contact a manager\n";

			string take;
			inputCheck(take, { "1","2","3","4"});
			int start = stoi(take);
			if (start == 1)
			{
				
				Customer customer;
				cout << "Please enter your ID: ";
				string inp;
				getline(cin, inp);

				while ((!isNum(inp) && inp != "b") || (isNum(inp) && !customer.retrieveFromDatabase(conn, stoi(inp))))
				{
					cout << "Enter a valid and existing User ID :"; getline(cin, inp);
				}
				if (inp == "b") continue;

				int userID = stoi(inp);
				cout << "Please enter your password: ";
				getline(cin, inp);
				while (inp != customer.getPassword() && inp != "b") {
					cout << "Wrong Password :( Enter again: ";  getline(cin, inp);
				}
				if (inp == "b") continue;
				
				cout << "Logged in Successfully with User ID " << userID << "\n";
				


				while (true)
				{
					
					cout << "Your customer record is: " << customer.getCustomerRecord() << "\n";
					cout << "You have currently fine " << customer.getFineDue() << " due\n";
					cout << "Press 1 to see available cars\n";
					cout << "Press 2 to view my rented cars\n";
					cout << "Press 3 to rent a car\n";
					cout << "Press 4 to return a car\n";
					cout << "Press 5 to clear dues\n";
					cout << "Press 6 to logout from the portal\n";

					inputCheck(inp, { "1","2","3","4","5","6" });
					if (inp == "1")
					{
						
						getAvailableCars(conn);

					}
					else if (inp == "2")
					{
						
						if (customer.getRentedCars().size() == 0) cout << "No cars have been rented currently\n";
						else getCarsByIds(conn, customer.getRentedCars());

					}
					else if (inp == "3")
					{
						
						if (customer.getCustomerRecord() / 40 <= customer.getRentedCars().size())
						{
							cout << "You cannot rent more cars :( \n";
							continue;
						}
						string check;
						Car car;
						cout << "Enter the Car ID of the required car: ";
						getline(cin, check);
						while (!isNum(check) || (isNum(check) && !car.retrieveFromDatabase(conn, stoi(check))) || car.getIsRented() == true)
						{
							if ((isNum(check) && car.retrieveFromDatabase(conn, stoi(check))) && car.getIsRented() == true)
								cout << "This car has already been rented\n" << "Enter a different Car ID: ";
							else cout << "Enter a valid Car ID: ";
							getline(cin, check);
						}



						cout << "Enter the due date of the car in dd mm yyyy format: ";
						string d, m, y;
						cin >> d >> m >> y;
						while ((!isNum(d) || !isNum(m) || !isNum(y))||!is_date_valid(stoi(d),stoi(m),stoi(y)))
						{
							cout << "Enter a valid date: ";
							cin >> d >> m >> y;
						}
						time_t now = time(0);
						tm* ltm = localtime(&now);

						int diff = getDifference({ ltm->tm_mday,1 + ltm->tm_mon,1900 + ltm->tm_year }, { stoi(d),stoi(m),stoi(y) });
						if (diff > 0)
						{
							cout << diff * (car.getCondition() * 100) << " is the cost for renting the car till the due date\n";
							car.request({ stoi(d),stoi(m),stoi(y) });
							car.updateToDatabase(conn);
							vector<int> rented;
							rented = customer.getRentedCars();
							rented.push_back(car.getId());
							customer.setRentedCars(rented);
						}
						else
						{
							cout << "Due Date cannot be of the past\n";
						}
					}
					else if (inp == "4")
					{
						
						string check;
						Car car;
						cout << "Enter the Car ID of the rented car: ";
						getline(cin, check);
						while (!isNum(check) || (isNum(check) && !car.retrieveFromDatabase(conn, stoi(check))))
						{
							if ((isNum(check) && car.retrieveFromDatabase(conn, stoi(check))) && car.getIsRented() == false)
								cout << "This car  rented\n" << "Enter a different Car ID: ";
							else cout << "Enter a valid Car ID: ";

							getline(cin, check);
						}
						int j = 0;
						for (int i = 0; i < customer.getRentedCars().size(); i++)
						{
							if (customer.getRentedCars()[i] == stoi(check)) j = 1;
						}
						if (j == 0)
						{
							cout << "This car is not rented by you\n";
						}
						else
						{
							cout << "Enter the return date of the car in dd mm yyyy format: ";
							string d, m, y;
							cin >> d >> m >> y;
							while ((!isNum(d) || !isNum(m) || !isNum(y)) || !is_date_valid(stoi(d), stoi(m), stoi(y)))
							{
								cout << "Enter a valid date: ";
								cin >> d >> m >> y;
							}
							cout << "Enter the current condition of the car (A value between 1 to 10): ";
							string bruh;
							cin >> bruh;
							while (!isNum(bruh) || (stoi(bruh) > 10 || stoi(bruh) < 1))
							{
								cout << "Enter a valid condition value: ";
								cin >> bruh;
							}

							customer.ReturnRequest(car, { stoi(d),stoi(m),stoi(y) }, stoi(bruh));
							car.setCondition(stoi(bruh));
							car.setIsRented(false);
							cout << "Your current due fine: " << customer.getFineDue() << "\n";
							cout << "Your current Customer Record is " << customer.getCustomerRecord() << "\n";
							car.updateToDatabase(conn);
						}
					}
					else if (inp == "5")
					{
						
						cout << "Your dues have been cleared :)\n";
						customer.setFineDue(0);
					}
					else if (inp == "6")
					{
						
						cout<<"Logged out successfully\n";
						break;
					}

                    if(inp!="6"&&inp!="1"&&inp!="2")
					customer.updateToDatabase(conn, customer);
				}

			}

			else if (start == 2)
			{
			    	
					Customer customer;
					cout << "Please enter your ID: ";
					string inp;
					getline(cin, inp);

					while ((!isNum(inp) && inp != "b") || (isNum(inp) && !customer.retrieveFromDatabaseEmp(conn, stoi(inp))))
					{
						cout << "Enter a valid and existing User ID :"; getline(cin, inp);
						
					}
					if (inp == "b") break;

					int userID = stoi(inp);
					cout << "Please enter your password: ";
					getline(cin, inp);
					while (inp != customer.getPassword() && inp != "b") {
						cout << "Wrong Password :( Enter again: ";  getline(cin, inp);
					}
					if (inp == "b") break;

					cout << "Logged in Successfully with User ID " << userID << "\n";
					

					
					while (true)
					{
						
						cout << "Your employee record is: " << customer.getCustomerRecord() << "\n";
						cout << "You have currently fine " << customer.getFineDue() << " due\n";
						cout << "Press 1 to see available cars\n";
						cout << "Press 2 to view my rented cars\n";
						cout << "Press 3 to rent a car\n";
						cout << "Press 4 to return a car\n";
						cout << "Press 5 to clear dues\n";
						cout << "Press 6 to logout from the portal\n";

						inputCheck(inp, { "1","2","3","4","5","6" });
						if (inp == "1")
						{
							
							getAvailableCars(conn);

						}
						else if (inp == "2")
						{
							
							if (customer.getRentedCars().size() == 0) cout << "No cars have been rented currently\n";
							else getCarsByIds(conn, customer.getRentedCars());

						}
						else if (inp == "3")
						{
							
							if (customer.getCustomerRecord() / 40 <= customer.getRentedCars().size())
							{
								cout << "You cannot rent more cars :( \n";
								continue;
							}
							string check;
							Car car;
							cout << "Enter the Car ID of the required car: ";
							getline(cin, check);
							while (check!="b"||!isNum(check) || (isNum(check) && !car.retrieveFromDatabase(conn, stoi(check))) || car.getIsRented() == true)
							{
								if ((isNum(check) && car.retrieveFromDatabase(conn, stoi(check))) && car.getIsRented() == true)
									cout << "This car has already been rented\n" << "Enter a different Car ID: ";
								else cout << "Enter a valid Car ID: ";
								getline(cin, check);
							}
							if (check == "b") break;



							cout << "Enter the due date of the car in dd mm yyyy format: ";
							string d, m, y;
							cin >> d >> m >> y;
							while ((!isNum(d) || !isNum(m) || !isNum(y)) || !is_date_valid(stoi(d), stoi(m), stoi(y)))
							{
								cout << "Enter a valid date: ";
								cin >> d >> m >> y;
							}
							time_t now = time(0);
							tm* ltm = localtime(&now);

							int diff = getDifference({ ltm->tm_mday,1 + ltm->tm_mon,1900 + ltm->tm_year }, { stoi(d),stoi(m),stoi(y) });
							if (diff > 0)
							{
								cout << diff * (car.getCondition() * 100) << " is the cost for renting the car till the due date\n";
								car.request({ stoi(d),stoi(m),stoi(y) });
								car.updateToDatabase(conn);
								vector<int> rented;
								rented = customer.getRentedCars();
								rented.push_back(car.getId());
								customer.setRentedCars(rented);
							}
							else
							{
								cout << "Due Date cannot be of the past\n";
							}
						}
						else if (inp == "4")
						{
							
							string check;
							Car car;
							cout << "Enter the Car ID of the rented car: ";
							getline(cin, check);
							while (check != "b" || !isNum(check) || (isNum(check) && !car.retrieveFromDatabase(conn, stoi(check))))
							{
								if ((isNum(check) && car.retrieveFromDatabase(conn, stoi(check))) && car.getIsRented() == false)
									cout << "This car  rented\n" << "Enter a different Car ID: ";
								else cout << "Enter a valid Car ID: ";

								getline(cin, check);
							}
							if (check == "b") break;
							int j = 0;
							for (int i = 0; i < customer.getRentedCars().size(); i++)
							{
								if (customer.getRentedCars()[i] == stoi(check)) j = 1;
							}
							if (j == 0)
							{
								cout << "This car is not rented by you\n";
							}
							else
							{
								cout << "Enter the return date of the car in dd mm yyyy format: ";
								string d, m, y;
								cin >> d >> m >> y;
								while ((!isNum(d) || !isNum(m) || !isNum(y)) || !is_date_valid(stoi(d), stoi(m), stoi(y)))
								{
									cout << "Enter a valid date: ";
									cin >> d >> m >> y;
								}
								cout << "Enter the current condition of the car (A value between 1 to 10): ";
								string bruh;
								cin >> bruh;
								while (!isNum(bruh) || (stoi(bruh) > 10 || stoi(bruh) < 1))
								{
									cout << "Enter a valid condition value: ";
									cin >> bruh;
								}

								customer.ReturnRequest(car, { stoi(d),stoi(m),stoi(y) }, stoi(bruh));
								car.setCondition(stoi(bruh));
								car.setIsRented(false);
								cout << "Your current due fine: " << customer.getFineDue() << "\n";
								cout << "Your current Customer Record is " << customer.getCustomerRecord() << "\n";
								car.updateToDatabase(conn);
							}
						}
						else if (inp == "5")
						{
						
							cout << "Your dues have been cleared :)\n";
							customer.setFineDue(0);
						}
						else if (inp == "6")
						{
			
							"Logged out successfully\n";
							break;
						}

						if(inp!="6"&&inp!="1"&&inp!="2")
						customer.updateToDatabaseEmp(conn, customer);
					}








			}
			else if (start == 3)
			{

				cout << "Enter Manager Password: ";
				string passw;
				getline(cin, passw);
				while (passw != "lmao_ded"&&passw!="b")
				{
					cout << "Wrong Password :(\n";
					cout << "Enter again: ";
					getline(cin, passw);
				}
				if (passw == "b") continue;

				while (true)
				{
				
				cout << "Press 1 to access cars database\n";
				cout << "Press 2 to access customers database\n";
				cout << "Press 3 to access employees database\n";
				cout << "Press 4 to logout\n";

				
					string inp;
					inputCheck(inp, { "1","2","3","4" });

					if (inp == "1")
					{

						while (true)
						{
							cout << "Press 1 to add a car\n";
							cout << "Press 2 to delete an existing car\n";
							cout << "Press 3 to view a car\n";
							cout << "Press 4 to modify a car\n";
							cout << "Press 5 to go back\n";
							
							string check1;
							inputCheck(check1, { "1","2","3","4","5"});
							if (check1 == "1")
							{
								
								cout << "Enter the model of the car to be added: ";
								string model1;
								getline(cin, model1);

								std::string insertCarQuery = "INSERT INTO Cars (Model) VALUES ('" + model1 + "')";
								conn->createStatement()->execute(insertCarQuery);

								sql::ResultSet* res = getLastRow(conn, "Cars");
								if (res && res->next()) {
									std::cout << "Car added in database with ID: " << res->getInt("Id") << std::endl;

								}
								else {
									std::cerr << "No rows found in the table." << std::endl;
								}
								delete res;
							}
							else if (check1 == "2")
							{
								
								cout << "Enter the Car ID to be deleted: ";
								string bruh;
								getline(cin, bruh);

								Car car;

								while ((!isNum(bruh) || (isNum(bruh) && !car.retrieveFromDatabase(conn, stoi(bruh))) || car.getIsRented() == true) && !(bruh == "b"))
								{
									if ((isNum(bruh) && car.retrieveFromDatabase(conn, stoi(bruh))) && car.getIsRented() == true)
										cout << "This car is under rent\n" << "Enter a different Car ID: ";
									else cout << "Enter a valid Car ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								if (deleteRow(conn, "Cars", stoi(bruh)))
								{
									cout << "Car with ID " << stoi(bruh) << " deleted successfully\n";
								}
								else cout << "Car not found\n";
							}
							else if (check1 == "3")
							{
								
								cout << "Enter the Car ID to be viewed: ";
								string bruh;
								getline(cin, bruh);

								Car car;

								while ((!isNum(bruh) || (isNum(bruh) && !car.retrieveFromDatabase(conn, stoi(bruh))) ) && !(bruh == "b"))
								{
									cout << "Enter a valid Car ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								cout << "Car ID: " << car.getId() << "\n";
								cout << "Car Condition: " << car.getCondition() << "\n";
								cout << "Is car under rent: " << car.getIsRented() << "\n";
								cout << "Car Model: " << car.getModel() << "\n";
								if (car.getIsRented() == true)
								{
									cout << "Car due date: " << car.getDueDate().d << " " << car.getDueDate().m << " " << car.getDueDate().y << "\n";
								}

							}
							else if (check1 == "4")
							{
							
								cout << "Enter the Car ID to be modified: ";
								string bruh;
								getline(cin, bruh);

								Car car;

								while ((!isNum(bruh) || (isNum(bruh) && !car.retrieveFromDatabase(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Car ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								cout << "Enter the current condition of the car to be updated: (A value between 1 to 10): ";
								cin >> bruh;
								while (!isNum(bruh) || (stoi(bruh) > 10 || stoi(bruh) < 1))
								{
									cout << "Enter a valid condition value: ";
									cin >> bruh;
								}
								
								car.setCondition(stoi(bruh));
								getline(cin, bruh);
								cout << "Enter the model of the car to be updated: ";
								getline(cin, bruh);
								car.setModel(bruh);
								car.updateToDatabase(conn);

							}
							else if (check1 == "5")
							{
								break;
							}

						}

						
					}
					else if (inp == "2")
					{
						while (true)
						{
							
							cout << "Press 1 to add a customer\n";
							cout << "Press 2 to delete an existing customer\n";
							cout << "Press 3 to view customer profile\n";
							cout << "Press 4 to modify customer attributes\n";
							cout << "Press 5 to go back\n";

							string check1;
							inputCheck(check1, { "1","2","3","4","5" });

							if (check1 == "1")
							{
							
								string name1, password1;
								cout << "Enter the name of the customer: ";
								getline(cin, name1);
								cout << "Enter password: ";
								getline(cin, password1);

								std::string insertCustomerQuery = "INSERT INTO Customers (Name, Password) VALUES ( '" + name1 + "', '" + password1 + "' )";
								conn->createStatement()->execute(insertCustomerQuery);
								sql::ResultSet* res = getLastRow(conn, "Customers");
								if (res && res->next()) {
									std::cout << "Customer added in database with ID: " << res->getInt("Id") << std::endl;

								}
								else {
									std::cerr << "No rows found in the table." << std::endl;
								}
							}
							else if (check1 == "2")
							{
								
								cout << "Enter the Customer ID to be deleted: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabase(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Customer ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								if (deleteRow(conn, "Customers", stoi(bruh)))
								{
									cout << "Customer with ID " << stoi(bruh) << "deleted successfully\n";
								}
								else cout << "Customer not found\n";
							}
							else if (check1 == "3")
							{
								
								cout << "Enter the Customer ID to be viewed: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabase(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Customer ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								cout << "Customer ID: "<<customer.getId()<<"\n";
								cout << "Customer Name: " << customer.getName()<<"\n";
								cout << "Customer Record: " << customer.getCustomerRecord()<<"\n";
								cout << "Fine due: " << customer.getFineDue()<<"\n";
								cout << "Cars rented by customer (Car IDs): ";
								for (int i = 0; i < customer.getRentedCars().size(); i++)
								{
									cout << customer.getRentedCars()[i] << " ";
								}
								cout << "\n";

							}
							else if (check1 == "4")
							{
								
								cout << "Enter the Customer ID to be modified: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabase(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Customer ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								while (true)
								{
									cout<<"Press 1 to update customer name \n";
									cout << "Press 2 to update customer record \n";
									cout << "Press 3 to update due fine \n";
									cout << "Press 4 to go back\n";
									string check2;
									inputCheck(check2, { "1","2","3","4" });
									{
										if (check2 == "1")
										{
											cout << "Enter the name to be updated: ";
											string name1;
											getline(cin, name1);
											customer.setName(name1);
											customer.updateToDatabase(conn, customer);
											
										}
										else if (check2 == "2")
										{
											cout << "Enter the new customer record (A value between 1 to 200): ";
											string bruh;
											while (!isNum(bruh) || (stoi(bruh) > 200 || stoi(bruh) < 1))
											{
												cout << "Enter a valid value: ";
												cin >> bruh;
											}
											customer.setCustomerRecord(stoi(bruh));
											customer.updateToDatabase(conn, customer);
										}
										else if (check2 == "3")
										{
											cout << "Enter the new fine due: ";
											string bruh;
											cin >> bruh;
											while (!isNum(bruh) || stoi(bruh) < 0)
											{
												cout << "Enter a valid value: ";
												cin >> bruh;
											}
											customer.setFineDue(stoi(bruh));
											customer.updateToDatabase(conn, customer);
										}
										else if (check2 == "4")
										{
											break;
										}
									}
								}
							}
							else if (check1 == "5") break;
						}

					}
					else if (inp == "3")
					{
						while (true)
						{
						
							cout << "Press 1 to add a employee\n";
							cout << "Press 2 to delete an existing employee\n";
							cout << "Press 3 to view employee profile\n";
							cout << "Press 4 to modify employee attributes\n";
							cout << "Press 5 to go back\n";

							string check1;
							inputCheck(check1, { "1","2","3","4","5" });

							if (check1 == "1")
							{
								
								string name1, password1;
								cout << "Enter the name of the employee: ";
								getline(cin, name1);
								cout << "Enter password: ";
								getline(cin, password1);

								std::string insertCustomerQuery = "INSERT INTO Employees (Name, Password) VALUES (  '" + name1 + "', '" + password1 + "' )";
								conn->createStatement()->execute(insertCustomerQuery);
								sql::ResultSet* res = getLastRow(conn, "Employees");
								if (res && res->next()) {
									std::cout << "Employee added in database with ID: " << res->getInt("Id") << std::endl;

								}
								else {
									std::cerr << "No rows found in the table." << std::endl;
								}
							}
							else if (check1 == "2")
							{
								
								cout << "Enter the Employee ID to be deleted: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabaseEmp(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Employee ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								if (deleteRow(conn, "Employees", stoi(bruh)))
								{
									cout << "Employee with ID " << stoi(bruh) << " deleted successfully\n";
								}
								else cout << "Employee not found\n";
							}
							else if (check1 == "3")
							{
								
								cout << "Enter the Employee ID to be viewed: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabaseEmp(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Employee ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								cout << "Employee ID: " << customer.getId() << "\n";
								cout << "Employee Name: " << customer.getName() << "\n";
								cout << "Employee Record: " << customer.getCustomerRecord() << "\n";
								cout << "Fine due: " << customer.getFineDue() << "\n";
								cout << "Cars rented by employee (Car IDs): ";
								for (int i = 0; i < customer.getRentedCars().size(); i++)
								{
									cout << customer.getRentedCars()[i] << ", ";
								}
								cout << "\n";

							}
							else if (check1 == "4")
							{
								
								cout << "Enter the Employee ID to be modified: ";
								string bruh;
								getline(cin, bruh);

								Customer customer;

								while ((!isNum(bruh) || (isNum(bruh) && !customer.retrieveFromDatabaseEmp(conn, stoi(bruh)))) && !(bruh == "b"))
								{
									cout << "Enter a valid Employee ID: ";
									getline(cin, bruh);
								}
								if (bruh == "b") break;
								while (true)
								{
									cout << "Press 1 to update employee name \n";
									cout << "Press 2 to update employee record \n";
									cout << "Press 3 to update due fine \n";
									cout << "Press 4 to go back\n";
									string check2;
									inputCheck(check2, { "1","2","3","4" });
									{
										if (check2 == "1")
										{
											cout << "Enter the name to be updated: ";
											string name1;
											getline(cin, name1);
											customer.setName(name1);
											customer.updateToDatabaseEmp(conn, customer);

										}
										else if (check2 == "2")
										{
											cout << "Enter the new employee record (A value between 1 to 200): ";
											string bruh;
											cin >> bruh;
											while (!isNum(bruh) || (stoi(bruh) > 200 || stoi(bruh) < 1))
											{
												cout << "Enter a valid value: ";
												cin >> bruh;
											}
											customer.setCustomerRecord(stoi(bruh));
											customer.updateToDatabaseEmp(conn, customer);
										}
										else if (check2 == "3")
										{
											cout << "Enter the new fine due: ";
											string bruh;
											cin >> bruh;
											while (!isNum(bruh) || stoi(bruh) < 0)
											{
												cout << "Enter a valid value: ";
												cin >> bruh;
											}
											customer.setFineDue(stoi(bruh));
											customer.updateToDatabaseEmp(conn, customer);
										}
										else if (check2 == "4")
										{
											break;
										}
									}
								}
							}
							else if (check1 == "5") break;
						}

						}
					else if (inp == "4") break;
					
					
					
					
					
					
				}

			}
			else if (start == 4)
			{
				break;

				}

		}

		delete conn;
		
	return 0;
}
