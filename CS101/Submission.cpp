#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
using namespace std;

const int MAX_ITEMS = 100;

void checkout(double &current_Order_Val, double &current_Order_Cal, int price, double cal, double &loyalty_Discount);

void bubblesort(int arr[], int s)
{
    if (s == 1)
        return;
    for (int j = 0; j < s - 1; j++)
    {
        if (arr[j] > arr[j + 1])
        {
            swap(arr[j], arr[j + 1]);
        }
    }
    bubblesort(arr, s - 1);
}

void recommendation(int &type, string filename, double &current_Order_Cal, double &current_Order_Val, double &loyalty_Discount)
{
    string line;
    fstream File;
    int count = 1;
    int serialNumbers[MAX_ITEMS];
    string foodNames[MAX_ITEMS];
    int prices[MAX_ITEMS];
    int calories[MAX_ITEMS];

    File.open("Desi_menu.txt", ios::in);

    File.seekg(0);
    while (getline(File, line) && count < MAX_ITEMS)
    {
        stringstream ss(line);
        string part1, part2, part3, part4;
        getline(ss, part1, '.');
        getline(ss, part2, '.');
        getline(ss, part3, '.');
        getline(ss, part4, '.');

        serialNumbers[count] = stoi(part1);
        foodNames[count] = part2;
        prices[count] = stoi(part3);
        calories[count] = stoi(part4);
        count++;
    }

    File.close();

    File.open("Fastfood_menu.txt", ios::in);

    File.seekg(0);
    while (getline(File, line) && count < MAX_ITEMS)
    {
        stringstream ss(line);
        string part1, part2, part3, part4;
        getline(ss, part1, '.');
        getline(ss, part2, '.');
        getline(ss, part3, '.');
        getline(ss, part4, '.');

        serialNumbers[count] = stoi(part1);
        foodNames[count] = part2;
        prices[count] = stoi(part3);
        calories[count] = stoi(part4);
        count++;
    }

    File.close();

    int a, b, n, randnum;
    a = 1;
    b = count;
    n = b - a; // not using -1 as count is already one aboove the actual number of entries

    srand(time(NULL));

    randnum = a + rand() % n;

    cout << "\nHere is a recommended dish: " << endl;

    cout << "\nFood Name: " << foodNames[randnum]
         << "\nPrice: Rs " << prices[randnum]
         << "\nCalories: " << calories[randnum] << endl;

    fstream ufile;

    ufile.open(filename, ios::app);

choice:
    char response;
    cout << "\nDo you want to order this: (y/n) " << endl;
    cin >> response;

    if (response == 'y' || response == 'Y')
    {
        ufile << "\nFood Name: " << foodNames[randnum]
              << "\nPrice: Rs " << prices[randnum]
              << "\nCalories: " << calories[randnum] << endl;
        checkout(current_Order_Val, current_Order_Cal, prices[randnum], calories[randnum], loyalty_Discount);
    response:
        cout << "Do you want to order anything else? (y/n)" << endl;
        cin >> response;

        if (response == 'y' || response == 'Y')
        {
            return;
        }
        else if (response == 'n' || response == 'N')
        {
            type = 3;
        }
        else
        {

            if (cin.fail())
            {                           // Check if input failed
                cin.clear();            // Clear the error flag
                cin.ignore(1000, '\n'); // Discard invalid input
                cout << "Invalid input. Please try again!" << endl;
            }
            goto response;
        }
    }
    else if (response == 'n' || response == 'N')
    {
        return;
    }
    else
    {
        if (cin.fail())
        {                           // Check if input failed
            cin.clear();            // Clear the error flag
            cin.ignore(1000, '\n'); // Discard invalid input
            cout << "Invalid input. Please try again!" << endl;
        }
        goto choice;
    }

    ufile.close();
}

void read_user_file(string filename, int &type, double &current_Order_Cal, double &current_Order_Val, double &loyalty_Discount)
{
    fstream ufile;
    double totalAmount = 0.0; // Total for the current session
    double grandTotal = 0.0;  // Total across all sessions

    ufile.open(filename, ios::in);
    string line;
    if (ufile.is_open())
    {
        if (!getline(ufile, line))
        {
            cout << "File is empty. No previous orders found.\n";
            ufile.close();
            // You can call `recommendation` here if you want to proceed even with an empty file
            recommendation(type, filename, current_Order_Cal, current_Order_Val, loyalty_Discount);
            return;
        }
        cout << "File found. Reading content:\n";
        string line;

        // Read and display the file's contents
        int count = 0;
        ufile.clear();
        ufile.seekg(0);
        while (getline(ufile, line))
        {
            cout << line << "\n";

            // Check if the line contains the "Price" keyword
            if (line.find("Price: Rs") ==0)
            {
                // Extract the price from the line
                stringstream ss(line);
                string temp, priceStr;
                int price;

                // Skip to the price part (e.g., "Rs 140")
                ss >> temp >> temp >> priceStr;

                // Convert the price string to a double
                price = stoi(priceStr);

                // Update the grand total
                grandTotal += price;
            }
            count++;
        }
        ufile.clear();
        ufile.seekg(0);

        count /= 4; // Divide by 4 as each order has 4 lines (Name, Price, Calories, etc.)
        int arr[count];

        int i = 0;

        // Parse prices for the current session
        while (getline(ufile, line))
        {
            if (line.find("Price: Rs") ==0)
            {
                stringstream ss(line);
                string temp, priceStr;
                int price;

                ss >> temp >> temp >> priceStr;

                price = stoi(priceStr);
                arr[i] = price;
                totalAmount += price; // Add to the current session total
                i++;
            }
        }

        bubblesort(arr, count);
        // cout << "\nSummary of This Session:\n";
        cout << "\nSmallest Order Value in Previous : Rs " << arr[0] << endl;
        cout << "\nLargest Order Value in Session: Rs " << arr[count - 1] << endl;
        if (grandTotal >= 5000)
            loyalty_Discount = 0.05;
        if (grandTotal >= 10000)
            loyalty_Discount = 0.10;
        if (grandTotal >= 15000)
            loyalty_Discount = 0.15;
        if (grandTotal >= 20000)
            loyalty_Discount = 0.20;

        cout << "\nGrand Total of All Previous Orders: Rs " << grandTotal << endl; // Display grand total
        ufile.close();
    }
    else
    {
        // File does not exist; create a new file
        ufile.open(filename, ios::out); // Open in write mode

        if (ufile.is_open())
        {
            cout << "File not found. Creating a new file: " << filename << "\n";
        }
        else
        {
            cerr << "Error creating the file.\n";
        }
    }

    ufile.close();

    recommendation(type, filename, current_Order_Cal, current_Order_Val, loyalty_Discount);
}

void display_menu(int &type, int serialNumbers[], string foodNames[], int prices[], int calories[], int &count)
{
    string menu;
    fstream File;

choice:
    cout << "Do you want to eat desi(1) or fastfood(2): \n(3) to exit the program." << endl;
    cin >> type;

    switch (type)
    {
    case 1:
        menu = "Desi_menu.txt";
        break;

    case 2:
        menu = "Fastfood_menu.txt";
        break;

    case 3:
        return;
        break;

    default:
        if (cin.fail())
        {                           // Check if input failed
            cin.clear();            // Clear the error flag
            cin.ignore(1000, '\n'); // Discard invalid input
            cout << "Invalid input. Please try again!" << endl;
        }
        goto choice;
    }

    File.open(menu, ios::in);

    if (!File)
    {
        cout << "Error opening file!" << endl;
        return;
    }

    string line;
    File.seekg(0);
    while (getline(File, line) && count < MAX_ITEMS)
    {
        stringstream ss(line);
        string part1, part2, part3, part4;

        getline(ss, part1, '.');
        getline(ss, part2, '.');
        getline(ss, part3, '.');
        getline(ss, part4, '.');

        serialNumbers[count] = stoi(part1);
        foodNames[count] = part2;
        prices[count] = stoi(part3);
        calories[count] = stoi(part4);
        count++;
    }

    File.close();
    cout << string(70, '-') << endl; // Separator line
    cout << "| " << setw(3) << left << "ID"
         << "| " << setw(25) << left << "Food Name"
         << " | " << setw(10) << right << "Price (Rs)"
         << "      | " << setw(8) << right << "Calories\n";

    cout << string(70, '-') << endl; // Separator line

    for (int i = 1; i < count; i++)
    {
        cout << setw(4) << left << serialNumbers[i] << " | "                // ID column
             << setw(25) << left << foodNames[i] << " | "                   // Food Name column
             << setw(10) << right << "Rs " << setw(5) << prices[i] << " | " // Price column
             << setw(5) << right << calories[i] << endl;                    // Calories column
    }
}

void place_order(string foodNames[], int prices[], int calories[], int &type, string filename, int &count, double &current_Order_Cal, double &current_Order_Val, double &loyalty_Discount)
{
    fstream ufile;

    ufile.open(filename, ios::app);

    int choice;

ID:
    cout << "\nWhat do you want to eat (Input ID): " << endl;
    cin >> choice;

    if (choice > count || choice < 1)
    {
        if (cin.fail())
        {                           // Check if input failed
            cin.clear();            // Clear the error flag
            cin.ignore(1000, '\n'); // Discard invalid input
            cout << "Invalid input. Please try again!" << endl;
        }
        goto ID;
    }
    else
    {
        cout << "\nFood Name: " << foodNames[choice]
             << "\nPrice: Rs " << prices[choice]
             << "\nCalories: " << calories[choice] << endl;

    choice:
        char response;
        cout << "Do you want to order this: (y/n) " << endl;
        cin >> response;

        if (response == 'y' || response == 'Y')
        {

            ufile << "\nFood Name: " << foodNames[choice]
                  << "\nPrice: Rs " << prices[choice]
                  << "\nCalories: " << calories[choice] << endl;
            checkout(current_Order_Val, current_Order_Cal, prices[choice], calories[choice], loyalty_Discount);
        response:
            cout << "Do you want to order anything else? (y/n)" << endl;
            cin >> response;

            if (response == 'y' || response == 'Y')
            {
                return;
            }
            else if (response == 'n' || response == 'N')
            {
                type = 3;
            }
            else
            {
                if (cin.fail())
                {                           // Check if input failed
                    cin.clear();            // Clear the error flag
                    cin.ignore(1000, '\n'); // Discard invalid input
                    cout << "Invalid input. Please try again!" << endl;
                }
                goto response;
            }
        }
        else if (response == 'n' || response == 'N')
        {
            return;
        }
        else
        {
            if (cin.fail())
            {                           // Check if input failed
                cin.clear();            // Clear the error flag
                cin.ignore(1000, '\n'); // Discard invalid input
                cout << "Invalid input. Please try again!" << endl;
            }
            goto choice;
        }
    }

    ufile.close();
}

void checkout(double &current_Order_Val, double &current_Order_Cal, int price, double cal, double &loyalty_Discount)
{
    // cout << price << endl;
    // cout << cal << endl;
    current_Order_Cal += cal;
    current_Order_Val += price;
}

void update_price(string menu, int itemSrNo, int newPrice)
{
    fstream file;
    file.open(menu, ios::in);

    if (!file)
    {
        cout << "Error: Could not open the menu file!" << endl;
        return;
    }

    string line;
    string tempFileContent;
    bool found = false;

    while (getline(file, line))
    {
        stringstream ss(line);
        string part1, part2, part3, part4;

        // Parse the line into its components
        getline(ss, part1, '.'); // Serial number
        getline(ss, part2, '.'); // Name
        getline(ss, part3, '.'); // Price
        getline(ss, part4, '.'); // Calories

        if (stoi(part1) == itemSrNo)
        {
            found = true;
            part3 = to_string(newPrice); // Update the price
        }

        // Reconstruct the updated line
        tempFileContent += part1 + "." + part2 + "." + part3 + "." + part4 + "\n";
    }

    file.close();

    if (!found)
    {
        cout << "Error: Menu item with SrNo " << itemSrNo << " not found!" << endl;
        return;
    }

    // Write the updated content back to the file
    ofstream outFile(menu, ios::trunc); // Open file in truncate mode to clear content
    if (outFile.is_open())
    {
        outFile << tempFileContent;
        outFile.close();
        cout << "Price updated successfully for Menu Item SrNo: " << itemSrNo << endl;
    }
    else
    {
        cout << "Error: Could not write to the menu file!" << endl;
    }
}

void new_dish()
{
    string dish_name, menu, line, line2;
    int dish_price, dish_cal, choice, dish_Id;
    char response;
    int line_count = 0;
    fstream file;

decision:
    cout << "Which file do you want to add the new dish? \nPress (1) for Desi and (2) for fast food" << endl;
    cin >> choice;

    switch (choice)
    {
    case 1:
        menu = "Desi_menu.txt";
        file.open(menu);

        while (getline(file, line))
        {
            line_count++;
        }
        file.close();

        break;

    case 2:
        menu = "Fastfood_menu.txt";
        file.open(menu);
        while (getline(file, line2))
        {
            line_count++;
        }
        file.close();

        break;

    default:
        if (cin.fail())
        {                           // Check if input failed
            cin.clear();            // Clear the error flag
            cin.ignore(1000, '\n'); // Discard invalid input
            cout << "Invalid input. Please try again!" << endl;
        }
        goto decision;
    }

    // cout << "Enter the ID of new dish : " << endl;
    // cin >> dish_Id;
    dish_Id = line_count + 1;
    cin.ignore();
    cout << "Enter the name of new dish : " << endl;
    getline(cin, dish_name);
    cout << "Enter the price of the dish: " << endl;
    cin >> dish_price;
    cout << "Enter the calories of the new dish: " << endl;
    cin >> dish_cal;

    line = to_string(dish_Id) + "." + dish_name + "." + to_string(dish_price) + "." + to_string(dish_cal);
    cout << line;
    fstream menufile;
    menufile.open(menu, ios::app);
    menufile << line << endl;
    cout << "\nItem added successfully in the file." << endl;
    menufile.close();

    cout << "Do you want to add any more items?(y/n)" << endl;
    cin >> response;

    if (response == 'y' || response == 'Y')
    {
        goto decision;
    }
    else
    {
        return;
    }
}

int main()
{
    string name, filename;
    string last4digits;
    long long int ph_number;
    const int MAX_ITEMS = 100;
    int serialNumbers[MAX_ITEMS];
    string foodNames[MAX_ITEMS];
    int prices[MAX_ITEMS];
    int calories[MAX_ITEMS];
    int count;
    int passcode;
    int adminpass = 221224;
    int role;
    char response;
    int type;
    char tip_Choice;
    double tip_Percentage;
    int payment_Choice;
    string menu;
    int itemSrNo;
    int newPrice;
    int a_choice;
    double current_Order_Val = 0.0;
    double current_Order_Cal = 0.0;
    double loyalty_Discount = 0.0;

    cout << "Welcome to MealMate-Pro" << endl;
a_role:
    cout << "Do you want to login as admin(1) or client(2)? " << endl;
    cin >> role;
    // cin.ignore();
    if (role != 1 && role != 2)
    {
        if (cin.fail() || role != 1 && role != 2)
        {                           // Check if input failed
            cin.clear();            // Clear the error flag
            cin.ignore(1000, '\n'); // Discard invalid input
            cout << "Invalid input. Please try again!" << endl;
        }
        goto a_role;
    }
    if (role == 2)
    {
        cin.ignore();
        cout << "Enter your name: " << endl;
        getline(cin, name);
        for (char &c : name)
        {
            c = tolower(c);
        }

        cout << "Enter your phone number: " << endl;
        cin >> ph_number;
        cin.ignore(); // Clear the input buffer

        last4digits = to_string(ph_number % 10000);
        cout << "Last 4 digits of phone number: " << last4digits << endl;

        filename = name + "_" + last4digits + ".txt";
        cout << "Filename: " << filename << endl;

        read_user_file(filename, type, current_Order_Cal, current_Order_Val, loyalty_Discount);

        if (type != 3)
        {
            do
            {
                count = 1;
                display_menu(type, serialNumbers, foodNames, prices, calories, count); 
                if (type != 3)
                {
                    place_order(foodNames, prices, calories, type, filename, count, current_Order_Cal, current_Order_Val, loyalty_Discount);
                }
            } while (type != 3);
        }
        if (type == 3 && current_Order_Cal == 0.0 && current_Order_Val == 0.0)
        {
            cout << "Exiting Program, Goodbye :) \n";
            return 0;
        }
        cout << "\n\nCurrent order calories: " << current_Order_Cal << "\n";
        cout << "Current Order value before GST: Rs " << current_Order_Val << "\n\n";
    t_choice:
        cout << "Do you want to give tip? (y/n)" << endl;
        cin >> tip_Choice;
        if (tip_Choice == 'y' || tip_Choice == 'Y')
        {
            cout << "Enter tip percentage: " << endl;
            cin >> tip_Percentage;
            tip_Percentage /= 100;
        }
        else if (tip_Choice != 'n' && tip_Choice != 'N')
        {
            if (cin.fail())
            {                           // Check if input failed
                cin.clear();            // Clear the error flag
                cin.ignore(1000, '\n'); // Discard invalid input
                cout << "Invalid input. Please try again!" << endl;
            }
            goto t_choice;
        }

    p_choice:
        cout << "Do you want to pay with cash(1) or card(2) ? " << endl;
        cin >> payment_Choice;

        if (payment_Choice == 2)
        {
            current_Order_Val *= 1.07;
            if (loyalty_Discount > 0)
                cout << "Order value before discount: " << current_Order_Val;
        }
        else if (payment_Choice == 1)
        {
            current_Order_Val *= 1.13;
            if (loyalty_Discount > 0)
                cout << "Order value before discount: " << current_Order_Val;
        }
        else
        {
            if (cin.fail())
            {                           // Check if input failed
                cin.clear();            // Clear the error flag
                cin.ignore(1000, '\n'); // Discard invalid input
                cout << "Invalid input. Please try again!" << endl;
            }
            goto p_choice;
        }
        if (loyalty_Discount > 0)
        {
            cout << "\nFor being our regular customer, we would like to offer you a loyalty discount of " << loyalty_Discount * 100 << " % Enjoy! :) \n";
            cout << "\nYour total order value after discount = " << current_Order_Val;
        }
        else
        {
            cout << "\n\nYour total order value = ";
            current_Order_Val = (1 - loyalty_Discount) * current_Order_Val;
            current_Order_Val *= (1 + tip_Percentage);
            cout << current_Order_Val;
        }
    }
    else
    {
        int pass_count = 0;
        fstream menufile;
        string line;
    role:

        if (role == 1)
        {
            int operation;
            int line_count = 0;
        pass:
            cout << "Enter passcode: " << endl;
            cin >> passcode;
            if (passcode == adminpass)
            {
                do
                {
                    cout << "What operation do you want to carry out? "
                         << "\n1. Update price of a dish"
                         << "\n2. Add a new item to a menu"
                         << "\nPress the number: ";
                    cin >> operation;

                    // Clear cin buffer in case of invalid input
                    if (cin.fail())
                    {
                        cin.clear();            // Clear error flag
                        cin.ignore(1000, '\n'); // Discard invalid input
                        operation = 0;          // Set to invalid operation
                    }
                } while (operation < 1 || operation > 2);

                switch (operation)
                {
                case 1:
                response:
                    cout << "Do you want to update price? (y/n) " << endl;
                    cin >> response;
                    if (response == 'y' || response == 'Y')
                    {
                    choice:
                        cout << "Which menu do you want to update? \nDesi(1) or Fastfood(2) " << endl;
                        cin >> a_choice;

                        switch (a_choice)
                        {
                        case 1:
                            menu = "Desi_menu.txt";
                            break;

                        case 2:
                            menu = "Fastfood_menu.txt";
                            break;

                        default:
                            if (cin.fail())
                            {                           // Check if input failed
                                cin.clear();            // Clear the error flag
                                cin.ignore(1000, '\n'); // Discard invalid input
                                cout << "Invalid input. Please try again!" << endl;
                            }
                            goto choice;
                        }
                        menufile.open(menu, ios::in);
                        cout << string(70, '-') << endl; // Separator line

                        cout << "| " << setw(3) << left << "ID"
                             << "| " << setw(25) << left << "Food Name"
                             << " | " << setw(10) << right << "Price (Rs)"
                             << "      | " << setw(8) << right << "Calories\n";

                        cout << string(70, '-') << endl; // Separator line

                        while (getline(menufile, line) && count < MAX_ITEMS)
                        {
                            stringstream ss(line);
                            string part1, part2, part3, part4;

                            getline(ss, part1, '.');
                            getline(ss, part2, '.');
                            getline(ss, part3, '.');
                            getline(ss, part4, '.');

                            cout << setw(4) << left << part1 << " | "                  // ID column
                                 << setw(25) << left << part2 << " | "                 // Food Name column
                                 << right << "Rs " << setw(5) << part3 << "        | " // Price column
                                 << setw(5) << right << part4 << endl;                 // Calories column
                            line_count++;
                        }
                        menufile.close();
                    ID:
                        char price_choice;
                        cout << "\nEnter the ID for which you want to update price: " << endl;
                        cin >> itemSrNo;
                        if (itemSrNo <= 0 || itemSrNo > line_count)
                        {
                            cout << "invalid ID, please try again" << endl;
                            goto ID;
                        }
                        cout << "Enter new price: " << endl;
                        cin >> newPrice;

                        update_price(menu, itemSrNo, newPrice);
                        cout << "Do you want to edit any other price? " << endl;
                        cin >> price_choice;
                        if (price_choice == 'y' || price_choice == 'Y')
                            goto ID;
                    }

                    else if (response == 'n' || response == 'N')
                    {
                        return 0;
                    }
                    else if (response != 'n' && response != 'N' && passcode == adminpass)
                    {
                        if (cin.fail())
                        {                           // Check if input failed
                            cin.clear();            // Clear the error flag
                            cin.ignore(1000, '\n'); // Discard invalid input
                            cout << "Invalid input. Please try again!" << endl;
                        }
                        goto response;
                    }
                    break;
                case 2:
                    new_dish();
                    break;
                }
            }

            else
            {
                cout << "Incorect passcode, are you an imposter? " << endl;

                pass_count++;
                if (pass_count > 3)
                {
                    cout << "Too many tries! \nExiting program\n";
                }
                else
                    goto pass;
            }
        }
    }

    return 0;
}