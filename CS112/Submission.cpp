#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <set>
#include "instructions.cpp"
#include <random>
#include <ctime>
#include <functional>
#include <vector>
#include <filesystem>
#include <regex>
#include <limits>
using namespace std;
namespace fs = std::filesystem;

// Forward declarations for news system
struct NewsItem {
    string title;
    string content;
    string category;
    string timestamp;
    bool isImportant;
};

vector<NewsItem> newsFeed;

void generateNews(const string& category, const string& title, const string& content, bool isImportant = false);
void generateRandomNews(int level, int ecoPoints, int pollutionLevel, const string& cityName);
string getCurrentTimeStr();

/**
 * @file code_wd_buildings.cpp
 * @brief Implementation of the Eco City game buildings and game mechanics
 * 
 * This file contains the implementation of various buildings in the Eco City game,
 * including their functionality, interactions, and game state management.
 */

// --------- Game Constants ---------
/**
 * @brief Cost and reward constants for various game actions
 */
const int HOUSE_EAT_COST = 20;              ///< Cost to eat at home
const int HOSPITAL_TREATMENT_COST = 50;     ///< Cost for hospital treatment
const int OFFICE_WORK_REWARD = 100;         ///< Reward for working at office
const int RESTAURANT_MEAL_COST = 50;        ///< Cost for restaurant meal
const int RESTAURANT_ECO_POINTS = 20;       ///< Eco points gained from restaurant
const int SCHOOL_STUDY_ECO_POINTS = 10;     ///< Eco points gained from studying
const int BANK_INTEREST_RATE = 2;           ///< Bank interest rate percentage
const int CASINO_BET_AMOUNT = 100;          ///< Amount to bet at casino
const int CASINO_WIN_REWARD = 200;          ///< Reward for winning at casino
const int HOUSE_RELAX_ECO_POINTS = 5;       ///< Eco points gained from relaxing at home

/**
 * @brief Health and hunger constants
 */
const int MAX_HEALTH = 100;                 ///< Maximum health value
const int MAX_HUNGER = 100;                 ///< Maximum hunger value
const int HUNGER_DECREASE_RATE = 2;         ///< Hunger decrease per minute
const int HEALTH_DECREASE_RATE = 1;         ///< Health decrease per minute
const int HEALTH_DECREASE_RATE_LOW_HUNGER = 5; ///< Health decrease per minute when hungry
const int HUNGER_THRESHOLD = 30;            ///< Hunger threshold for increased health decrease
const int HOSPITAL_HEALTH_RESTORE = 30;     ///< Health restored at hospital
const int HOUSE_SLEEP_HEALTH_RESTORE = 20;  ///< Health restored while sleeping
const int HOUSE_EAT_HUNGER_RESTORE = 30;    ///< Hunger restored while eating

/**
 * @brief Level and pollution constants
 */
const int LEVEL_POINTS_PER_ECO = 1;         ///< Level points per eco point
const int POLLUTION_PENALTY = 2;            ///< Pollution penalty multiplier
const int LEVEL_UP_THRESHOLD = 100;         ///< Level points needed to level up

/**
 * @brief Win condition constants
 */
const int WIN_REQUIRED_LEVEL = 10;          ///< Required level to win
const int WIN_REQUIRED_DAYS = 5;            ///< Required days with zero pollution to win

/**
 * @brief Transportation delay constants in seconds
 */
const int WALK_DELAY = 10;                  ///< Walking delay in seconds
const int CYCLE_DELAY = 5;                  ///< Cycling delay in seconds
const int CAR_DELAY = 0;                    ///< Car delay in seconds

/**
 * @brief Periodic reward constants
 */
const int PERIODIC_FUNDS_REWARD = 10;       ///< Funds gained periodically
const int PERIODIC_FUNDS_INTERVAL = 60;     ///< Interval for periodic rewards in seconds
const int OFFLINE_BONUS_PER_DAY = 1000;     ///< Offline bonus per day

// Update Transport Constants
const int CYCLE_COST = 100;
const int CAR_COST = 300;
const int ELECTRIC_CAR_COST = 500;
const int CYCLE_REQUIRED_LEVEL = 2;
const int CAR_REQUIRED_LEVEL = 4;
const int ELECTRIC_CAR_REQUIRED_LEVEL = 6;
const int CAR_ECO_PENALTY = 5;
const int CAR_POLLUTION_INCREASE = 10;
const int ELECTRIC_CAR_ECO_BONUS = 5;

// New Restaurant Constants
const int BASIC_MEAL_COST = 10;
const int BASIC_MEAL_HUNGER = 10;
const int BASIC_MEAL_ECO_PENALTY = 5;

const int INTERMEDIATE_MEAL_COST = 25;
const int INTERMEDIATE_MEAL_HUNGER = 20;

const int PREMIUM_MEAL_COST = 50;
const int PREMIUM_MEAL_ECO_POINTS = 20;
const int PREMIUM_MEAL_HUNGER = 35;

// New Environment Constants
const int TREE_COST = 10;
const int TREE_ECO_POINTS = 10;
const int TREE_POLLUTION_REDUCTION = 10;

// Update Transport System
bool hasBicycle = false;
bool hasCar = false;
bool hasElectricCar = false;

// Add House Relaxation Constant
const int HOUSE_RELAX_TIME_ACCELERATION = 2;  ///< Time acceleration factor while relaxing

// Add Summit Constants
const int SUMMIT_REQUIRED_LEVEL = 3;
const int SUMMIT_ECO_POINTS = 50;
const int SUMMIT_FUNDS_COST = 200;
const int LOAN_INTEREST_RATE = 5;  // 5% interest on loans

// --------- Loan Enhancements ---------
const int LOAN_DAYS_BEFORE_PENALTY = 7;
const int LOAN_LATE_PENALTY = 10; // extra 10% interest if late

struct LoanInfo {
    bool hasLoan = false;
    int loanAmount = 0;
    string lenderName = "";
    string loanStartDate = "";
};

string getCurrentDateStrLoan() {
    time_t t = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}

int daysBetween(const string& start, const string& end) {
    tm tm1 = {}, tm2 = {};
    strptime(start.c_str(), "%Y-%m-%d", &tm1);
    strptime(end.c_str(), "%Y-%m-%d", &tm2);
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return (t2 - t1) / (60 * 60 * 24);
}

void notifyLenderOfRepayment(const string& lenderFile, const string& borrower) {
    ofstream out(lenderFile, ios::app);
    if (out.is_open()) {
        out << "[Notification] Loan repaid by " << borrower << " on " << getCurrentDateStrLoan() << "\n";
        out.close();
    }
}

// --------- Function Declarations ---------
void clearScreen();
void displayStatusBar(int value, int max, const string& label, const string& color);
void displayStatus(int funds, int health, int hunger, int level, int levelPoints, int ecoPoints, int pollutionLevel, const string& cityName);
void updateLevel(int& level, int& levelPoints, int ecoPoints, int pollutionLevel);
void updateHungerAndHealth(int& health, int& hunger, time_t& lastUpdateTime);
void transport_delay(int vehicle, int& ecoPoints, int& pollutionLevel);
void update_funds_periodically(int &funds, time_t &lastUpdateTime);
void offline_bonus(int &funds, time_t &lastSaveTime);
void save_game(string filename, int level, int ecopoints, int funds, int health, int hunger,
    int houseUp, int hospitalUp, int officeUp, int restaurantUp, int schoolUp, int bankUp, int casinoUp, int envUp, int recUp, int gardenUp,
    int vehicle, int pollutionlevel, time_t lastSaveTime, int levelPoints, bool hasLoan, int loanAmount, string lenderName, string loanStartDate, string cityName, int daysWithZeroPollution);
bool load_game(string filename, int &level, int &ecopoints, int &funds, int &health, int &hunger,
    int &houseUp, int &hospitalUp, int &officeUp, int &restaurantUp, int &schoolUp, int &bankUp, int &casinoUp, int &envUp, int &recUp, int &gardenUp,
    int& vehicle, int& pollutionlevel, time_t& lastSaveTime, int& levelPoints,
    bool &hasLoan, int &loanAmount, string &lenderName, string &loanStartDate, string &cityName, int& daysWithZeroPollution);

// --------- ASCII Art for Buildings ---------
const string HOUSE_ART = R"(
  /\  
 /  \   _
/____\_|_|
| []  [] |
|   __   |
|__|  |__|
)";
const string HOSPITAL_ART = R"(
  _____
 |  +  |
 |_____| 
 |     |
 |_____| 
)";
const string OFFICE_ART = R"(
  _______
 |  _  _ |
 | | | ||
 | | | ||
 |_____/|
)";
const string RESTAURANT_ART = R"(
   _____
  /     \
 |  o o |
 |  --- |
 |_____/ 
)";
const string SCHOOL_ART = R"(
   _____
  /     \
 |  S S |
 |  --- |
 |_____/ 
)";
const string BANK_ART = R"(
  _______
 | $   $ |
 |_______|
 |       |
 |_______|
)";
const string CASINO_ART = R"(
  _______
 | 77777|
 |_______|
 |       |
 |_______|
)";
const string ENVIRONMENT_ART = R"(
   _/\_
  /    \
  \    /
   \  /
    \/
   (🌳)
)";
const string RECYCLING_ART = R"(
   _____
  /     \
 |  ♻️  |
 |      |
 |_____/ 
)";
const string GARDEN_ART = R"(
   _/\_
  (🌻🌻)
  (🌻🌻)
   \  /
    \/
)";

// --------- Building Upgrades ---------
struct BuildingUpgrade {
    int level = 1;
    static const int maxLevel = 3;
    int upgradeCost() const { return 200 * level; }
    bool canUpgrade() const { return level < maxLevel; }
    void upgrade(int& funds) {
        if (canUpgrade() && funds >= upgradeCost()) {
            funds -= upgradeCost();
            level++;
            cout << "Building upgraded to level " << level << "!\n";
        } else if (!canUpgrade()) {
            cout << "Already at max level!\n";
        } else {
            cout << "Not enough funds to upgrade!\n";
        }
    }
};

// --------- Achievements System ---------
struct Achievements {
    set<string> unlocked;
    set<string> allAchievements = {
        // Environmental Achievements
        "Tree Planter: Planted 10 trees",
        "Forest Guardian: Planted 50 trees",
        "Pollution Fighter: Reduced pollution to 0",
        "Eco Warrior: Reached 1000 eco points",
        "Green Thumb: Planted 100 trees",
        
        // Building Achievements
        "Architect: Upgraded any building to level 3",
        "Master Builder: Upgraded all buildings to level 3",
        "City Planner: Upgraded 5 different buildings",
        
        // Summit & Social Achievements
        "Summit Leader: Hosted 3 summits",
        "Diplomat: Hosted 10 summits",
        "Community Builder: Hosted 5 summits",
        
        // Financial Achievements
        "Debt-Free: Repaid a loan",
        "Wealthy Citizen: Accumulated 10,000 funds",
        "Financial Expert: Earned 1,000 funds in interest",
        "Business Tycoon: Accumulated 50,000 funds",
        
        // Transport Achievements
        "Eco Commuter: Purchased an electric car",
        "Green Traveler: Used bicycle 50 times",
        "Transport Master: Owned all vehicle types",
        
        // Health & Wellbeing Achievements
        "Healthy Citizen: Maintained max health for 5 days",
        "Well Fed: Maintained max hunger for 5 days",
        "Life Balance: Kept both health and hunger above 80 for 3 days",
        
        // Special Achievements
        "Eco City Champion: Reached level 10",
        "Sustainability Expert: Achieved zero pollution for 5 days",
        "Master Gardener: Planted 200 trees"
    };
    
    // Achievement progress tracking
    int treesPlanted = 0;
    int summitsHosted = 0;
    int loansRepaid = 0;
    int buildingsUpgraded = 0;
    int maxBuildingsUpgraded = 0;
    int daysWithMaxHealth = 0;
    int daysWithMaxHunger = 0;
    int daysWithHighStats = 0;
    int bicycleUses = 0;
    int totalFundsEarned = 0;
    int interestEarned = 0;
    
    void checkAndUnlock() {
        // Environmental Achievements
        if (treesPlanted >= 10) unlocked.insert("Tree Planter: Planted 10 trees");
        if (treesPlanted >= 50) unlocked.insert("Forest Guardian: Planted 50 trees");
        if (treesPlanted >= 100) unlocked.insert("Green Thumb: Planted 100 trees");
        if (treesPlanted >= 200) unlocked.insert("Master Gardener: Planted 200 trees");
        
        // Summit Achievements
        if (summitsHosted >= 3) unlocked.insert("Summit Leader: Hosted 3 summits");
        if (summitsHosted >= 5) unlocked.insert("Community Builder: Hosted 5 summits");
        if (summitsHosted >= 10) unlocked.insert("Diplomat: Hosted 10 summits");
        
        // Financial Achievements
        if (loansRepaid >= 1) unlocked.insert("Debt-Free: Repaid a loan");
        if (totalFundsEarned >= 10000) unlocked.insert("Wealthy Citizen: Accumulated 10,000 funds");
        if (interestEarned >= 1000) unlocked.insert("Financial Expert: Earned 1,000 funds in interest");
        if (totalFundsEarned >= 50000) unlocked.insert("Business Tycoon: Accumulated 50,000 funds");
        
        // Transport Achievements
        if (bicycleUses >= 50) unlocked.insert("Green Traveler: Used bicycle 50 times");
        
        // Health Achievements
        if (daysWithMaxHealth >= 5) unlocked.insert("Healthy Citizen: Maintained max health for 5 days");
        if (daysWithMaxHunger >= 5) unlocked.insert("Well Fed: Maintained max hunger for 5 days");
        if (daysWithHighStats >= 3) unlocked.insert("Life Balance: Kept both health and hunger above 80 for 3 days");
        
        // Building Achievements
        if (maxBuildingsUpgraded >= 1) unlocked.insert("Architect: Upgraded any building to level 3");
        if (buildingsUpgraded >= 5) unlocked.insert("City Planner: Upgraded 5 different buildings");
        if (maxBuildingsUpgraded >= 10) unlocked.insert("Master Builder: Upgraded all buildings to level 3");
    }
    
    void display() {
        cout << "\n🏅 Achievements & Badges 🏅\n";
        if (unlocked.empty()) {
            cout << "No achievements unlocked yet.\n";
        }
        
        // Group achievements by category
        map<string, vector<string>> categories = {
            {"🌱 Environmental", {}},
            {"🏢 Buildings", {}},
            {"🤝 Summit & Social", {}},
            {"💰 Financial", {}},
            {"🚗 Transport", {}},
            {"❤️ Health & Wellbeing", {}},
            {"🏆 Special", {}}
        };
        
        // Sort achievements into categories
        for (const auto& ach : allAchievements) {
            if (ach.find("Tree") != string::npos || ach.find("Pollution") != string::npos || 
                ach.find("Eco") != string::npos || ach.find("Green") != string::npos) {
                categories["🌱 Environmental"].push_back(ach);
            }
            else if (ach.find("building") != string::npos || ach.find("Architect") != string::npos) {
                categories["🏢 Buildings"].push_back(ach);
            }
            else if (ach.find("Summit") != string::npos || ach.find("Community") != string::npos || 
                     ach.find("Diplomat") != string::npos) {
                categories["🤝 Summit & Social"].push_back(ach);
            }
            else if (ach.find("funds") != string::npos || ach.find("Debt") != string::npos || 
                     ach.find("Wealthy") != string::npos || ach.find("Financial") != string::npos) {
                categories["💰 Financial"].push_back(ach);
            }
            else if (ach.find("car") != string::npos || ach.find("bicycle") != string::npos || 
                     ach.find("Transport") != string::npos) {
                categories["🚗 Transport"].push_back(ach);
            }
            else if (ach.find("health") != string::npos || ach.find("hunger") != string::npos || 
                     ach.find("Well") != string::npos) {
                categories["❤️ Health & Wellbeing"].push_back(ach);
            }
            else {
                categories["🏆 Special"].push_back(ach);
            }
        }
        
        // Display achievements by category
        for (const auto& category : categories) {
            if (!category.second.empty()) {
                cout << "\n" << category.first << ":\n";
                for (const auto& ach : category.second) {
                    if (unlocked.count(ach) > 0) {
                        cout << "✅ " << ach << "\n";
                    } else {
                        cout << "🔒 " << ach << "\n";
                    }
                }
            }
        }
        cout << endl;
    }
};
Achievements achievements;

// --------- Challenges System ---------
struct Challenge {
    string description;
    int target;
    int progress;
    bool completed;
    string rewardType; // "funds" or "eco"
    int rewardAmount;
    string type; // "daily" or "weekly"
    string id; // unique id for challenge
    string lastResetDate;
};
Challenge dailyChallenge = {"Plant 5 trees today", 5, 0, false, "funds", 100, "daily", "plant5trees", ""};
Challenge weeklyChallenge = {"Host 2 summits this week", 2, 0, false, "eco", 50, "weekly", "host2summits", ""};
string getCurrentDateStr() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}
string getCurrentWeekStr() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-W%V", localtime(&t));
    return string(buf);
}
void resetChallengeIfNeeded(Challenge &ch) {
    string today = (ch.type == "daily") ? getCurrentDateStr() : getCurrentWeekStr();
    if (ch.lastResetDate != today) {
        ch.progress = 0;
        ch.completed = false;
        ch.lastResetDate = today;
    }
}
void checkChallengeCompletion(Challenge &ch, int &funds, int &ecoPoints) {
    if (!ch.completed && ch.progress >= ch.target) {
        ch.completed = true;
        if (ch.rewardType == "funds") funds += ch.rewardAmount;
        if (ch.rewardType == "eco") ecoPoints += ch.rewardAmount;
        cout << "\n🎉 Challenge completed: " << ch.description << "! Reward: ";
        if (ch.rewardType == "funds") cout << ch.rewardAmount << " funds";
        if (ch.rewardType == "eco") cout << ch.rewardAmount << " eco points";
        cout << "\n";
    }
}
void showChallenges() {
    cout << "\n📅 Challenges:\n";
    cout << "- Daily:   " << dailyChallenge.description << " (" << dailyChallenge.progress << "/" << dailyChallenge.target << ") ";
    if (dailyChallenge.completed) cout << "✅";
    cout << "\n- Weekly:  " << weeklyChallenge.description << " (" << weeklyChallenge.progress << "/" << weeklyChallenge.target << ") ";
    if (weeklyChallenge.completed) cout << "✅";
    cout << "\n";
}

// --------- Player-to-Player Messaging ---------
void leaveMessage(const string& recipientFile, const string& sender, const string& context) {
    string msg;
    cout << "Enter a short message for " << recipientFile.substr(0, recipientFile.find('_')) << " (or press Enter to skip): ";
    getline(cin, msg);
    if (!msg.empty()) {
        ofstream out(recipientFile, ios::app);
        if (out.is_open()) {
            out << "[Message from " << sender << " about " << context << "]: " << msg << "\n";
            out.close();
        }
    }
}

// --------- Building Classes ---------
/**
 * @class House
 * @brief Represents the player's house in the game
 * 
 * The house provides basic activities like sleeping, eating, and relaxing.
 * It affects both funds and eco points.
 */
class House {
    private:
        int& funds;
        int& ecoPoints;
        int& health;
        int& hunger;
        int& level;
        int& levelPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
    public:
        House(int& f, int& e, int& h, int& hu, int& l, int& lp, int& p, const string& cn, int up=1) 
            : funds(f), ecoPoints(e), health(h), hunger(hu), level(l), levelPoints(lp), pollutionLevel(p), cityName(cn) { upgrade.level = up; }

        void enter() {
            clearScreen();
            cout << HOUSE_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🏠 Welcome to your House (Level " << upgrade.level << ")!\n";
                cout << "1. 🛌 Sleep (Restores " << (HOUSE_SLEEP_HEALTH_RESTORE * upgrade.level) << " health)\n";
                cout << "2. 🍴 Eat (Costs " << HOUSE_EAT_COST << " funds, Restores " << (HOUSE_EAT_HUNGER_RESTORE * upgrade.level) << " hunger)\n";
                cout << "3. 🎮 Relax (+" << (HOUSE_RELAX_ECO_POINTS * upgrade.level) << " ecoPoints)\n";
                if (upgrade.canUpgrade()) cout << "4. Upgrade House (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "5. 🚪 Exit House\n";
                cout << "Choose an option: ";
                int choice;
                cin >> choice;
                cin.ignore();
                switch (choice) {
                    case 1:
                        sleep();
                        break;
                    case 2:
                        eat();
                        break;
                    case 3:
                        relax();
                        break;
                    case 4:
                        if (upgrade.canUpgrade()) upgrade.upgrade(funds);
                        else cout << "Invalid option. Try again!\n";
                        break;
                    case 5:
                        cout << "🚶 Leaving House...\n";
                        clearScreen();
                        return;
                    default:
                        cout << "Invalid option. Try again!\n";
                }
            }
        }
    
    private:
        void sleep() {
            cout << "😴 Sleeping... (5 seconds)\n";
            this_thread::sleep_for(chrono::seconds(5));
            health = min(health + HOUSE_SLEEP_HEALTH_RESTORE * upgrade.level, MAX_HEALTH);
            cout << "You feel refreshed! (Health: " << health << "/" << MAX_HEALTH << ")\n";
        }
    
        void eat() {
            if (funds >= HOUSE_EAT_COST) {
                funds -= HOUSE_EAT_COST;
                hunger = min(hunger + HOUSE_EAT_HUNGER_RESTORE * upgrade.level, MAX_HUNGER);
                ecoPoints += HOUSE_RELAX_ECO_POINTS * upgrade.level;
                updateLevel(level, levelPoints, ecoPoints, pollutionLevel);
                cout << "You ate a meal and restored " << HOUSE_EAT_HUNGER_RESTORE << " hunger points!\n";
            } else {
                cout << "❌ Not enough funds!\n";
            }
        }
    
        void relax() {
            cout << "🎮 Relaxing... (2 seconds)\n";
            this_thread::sleep_for(chrono::seconds(2));
            ecoPoints += HOUSE_RELAX_ECO_POINTS * upgrade.level;
            updateLevel(level, levelPoints, ecoPoints, pollutionLevel);
            cout << "You gained " << (HOUSE_RELAX_ECO_POINTS * upgrade.level) << " eco points!\n";
        }
    public:
        int getUpgradeLevel() const { return upgrade.level; }
    };

/**
 * @class Hospital
 * @brief Represents the hospital building
 * 
 * The hospital provides treatment services that cost funds.
 */
class Hospital {
private:
    int& funds;
    int& health;
    int& hunger;
    int& level;
    int& levelPoints;
    int& ecoPoints;
    int& pollutionLevel;
    BuildingUpgrade upgrade;
    const string& cityName;
public:
    Hospital(int& f, int& h, int& hu, int& l, int& lp, int& e, int& p, const string& cn, int up=1) 
        : funds(f), health(h), hunger(hu), level(l), levelPoints(lp), ecoPoints(e), pollutionLevel(p), cityName(cn) { upgrade.level = up; }

    void enter() {
        clearScreen();
        cout << HOSPITAL_ART;
        while (true) {
            displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
            cout << "\n🏥 Welcome to the Hospital (Level " << upgrade.level << ")!\n";
            cout << "1. 💉 Get Treatment (Costs " << HOSPITAL_TREATMENT_COST << " funds, Restores " << (HOSPITAL_HEALTH_RESTORE * upgrade.level) << " health)\n";
            if (upgrade.canUpgrade()) cout << "2. Upgrade Hospital (Cost: " << upgrade.upgradeCost() << " funds)\n";
            cout << "3. 🚪 Exit Hospital\n";
            cout << "Choose an option: ";
            int choice;
            cin >> choice;
            cin.ignore();
            if (choice == 1) {
                if (funds >= HOSPITAL_TREATMENT_COST) {
                    cout << "💉 Treating... (3 seconds)\n";
                    funds -= HOSPITAL_TREATMENT_COST;
                    health = min(health + HOSPITAL_HEALTH_RESTORE * upgrade.level, MAX_HEALTH);
                    this_thread::sleep_for(chrono::seconds(3));
                    cout << "You feel much better! (Health: " << health << "/" << MAX_HEALTH << ", Funds left: " << funds << ")\n";
                } else {
                    cout << "❌ Not enough funds!\n";
                }
            } else if (choice == 2 && upgrade.canUpgrade()) {
                upgrade.upgrade(funds);
            } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                cout << "🚶 Leaving Hospital...\n";
                clearScreen();
                return;
            } else {
                cout << "Invalid option. Try again.\n";
            }
        }
    }

    int getUpgradeLevel() const { return upgrade.level; }
};

/**
 * @class Office
 * @brief Represents the office building
 * 
 * The office allows players to work and earn funds.
 */
class Office {
    private:
        int& funds;
        int& health;
        int& hunger;
        int& level;
        int& levelPoints;
        int& ecoPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
        
        struct MathProblem {
            string question;
            int answer;
            int difficulty;  // 1-3 for different difficulty levels
        };
        
        vector<MathProblem> problems = {
            // Easy problems (difficulty 1)
            {"What is 5 + 7?", 12, 1},
            {"What is 10 - 4?", 6, 1},
            {"What is 3 × 4?", 12, 1},
            {"What is 15 ÷ 3?", 5, 1},
            {"What is 8 + 9?", 17, 1},
            {"What is 6 × 7?", 42, 1},
            {"What is 20 - 8?", 12, 1},
            {"What is 24 ÷ 6?", 4, 1},
            {"What is 11 + 13?", 24, 1},
            {"What is 9 × 5?", 45, 1},
            
            // Easy word problems (difficulty 1)
            {"If you have 7 apples and buy 5 more, how many do you have?", 12, 1},
            {"A box has 8 chocolates. If you eat 3, how many are left?", 5, 1},
            {"There are 4 rows of chairs with 6 chairs each. How many chairs total?", 24, 1},
            {"You have 15 candies to share among 3 friends. How many each?", 5, 1},
            {"A bus has 9 passengers and 4 more get on. How many now?", 13, 1},
            
            // Medium problems (difficulty 2)
            {"What is 25 × 4?", 100, 2},
            {"What is 72 ÷ 8?", 9, 2},
            {"What is 15 × 6?", 90, 2},
            {"What is 100 - 37?", 63, 2},
            {"What is 13 × 7?", 91, 2},
            {"What is 48 × 3?", 144, 2},
            {"What is 120 ÷ 5?", 24, 2},
            {"What is 85 - 29?", 56, 2},
            {"What is 16 × 8?", 128, 2},
            {"What is 99 ÷ 9?", 11, 2},
            
            // Medium word problems (difficulty 2)
            {"A store sells shirts for $25 each. How much for 4 shirts?", 100, 2},
            {"You have $72 and want to buy books at $8 each. How many can you buy?", 9, 2},
            {"A garden has 15 rows of flowers with 6 in each row. How many flowers?", 90, 2},
            {"You have $100 and spend $37. How much is left?", 63, 2},
            {"A box contains 13 packs of cards with 7 cards each. How many cards?", 91, 2},
            {"A train travels 48 km per hour. How far in 3 hours?", 144, 2},
            {"You have 120 cookies to share among 5 friends. How many each?", 24, 2},
            {"A store had 85 items and sold 29. How many remain?", 56, 2},
            {"A classroom has 16 rows with 8 students each. How many students?", 128, 2},
            {"99 students need to be divided into 9 equal groups. How many per group?", 11, 2},
            
            // Hard problems (difficulty 3)
            {"What is 17 × 23?", 391, 3},
            {"What is 256 ÷ 16?", 16, 3},
            {"What is 45 × 45?", 2025, 3},
            {"What is 1000 - 387?", 613, 3},
            {"What is 19 × 21?", 399, 3},
            {"What is 34 × 28?", 952, 3},
            {"What is 512 ÷ 32?", 16, 3},
            {"What is 750 - 283?", 467, 3},
            {"What is 27 × 31?", 837, 3},
            {"What is 625 ÷ 25?", 25, 3},
            
            // Hard word problems (difficulty 3)
            {"A factory produces 17 items per hour for 23 hours. How many items?", 391, 3},
            {"256 students need to be divided into 16 equal classes. How many per class?", 16, 3},
            {"A square field is 45 meters on each side. What is its area?", 2025, 3},
            {"You have $1000 and spend $387. How much is left?", 613, 3},
            {"A store has 19 shelves with 21 items each. How many items total?", 399, 3},
            {"A train travels 34 km per hour for 28 hours. How far does it go?", 952, 3},
            {"512 cookies need to be packed into boxes of 32. How many boxes?", 16, 3},
            {"A store had 750 items and sold 283. How many remain?", 467, 3},
            {"A garden has 27 rows with 31 plants each. How many plants?", 837, 3},
            {"625 students need to be divided into 25 equal groups. How many per group?", 25, 3},
            
            // Environmental word problems (all difficulties)
            {"You plant 7 trees each day for 5 days. How many trees total?", 35, 1},
            {"A recycling center processes 8 tons of waste per hour. How much in 6 hours?", 48, 2},
            {"A solar panel produces 45 kWh per day. How much energy in 45 days?", 2025, 3},
            {"You collect 12 plastic bottles each day. How many in a week?", 84, 1},
            {"A wind turbine generates 25 kWh per hour. How much in 4 hours?", 100, 2},
            {"A forest has 17 trees per row and 23 rows. How many trees?", 391, 3},
            {"You recycle 15 items each day. How many in 6 days?", 90, 1},
            {"A solar farm has 16 panels producing 8 kWh each. Total energy?", 128, 2},
            {"A city reduces pollution by 34% from 750 units. How much remains?", 495, 3},
            {"You plant 9 trees each week. How many in 5 weeks?", 45, 1}
        };
        
        MathProblem getRandomProblem() {
            static default_random_engine rng(static_cast<unsigned>(time(nullptr)));
            
            // Filter problems based on office level
            vector<MathProblem> availableProblems;
            int maxDifficulty = min(3, upgrade.level);  // Max difficulty based on office level
            
            for (const auto& problem : problems) {
                if (problem.difficulty <= maxDifficulty) {
                    availableProblems.push_back(problem);
                }
            }
            
            uniform_int_distribution<size_t> dist(0, availableProblems.size() - 1);
            return availableProblems[dist(rng)];
        }
        
        bool solveMathProblem() {
            MathProblem problem = getRandomProblem();
            cout << "\n📊 Solve this problem to earn your salary:\n";
            cout << problem.question << "\n";
            cout << "Your answer: ";
            
            int answer;
            cin >> answer;
            cin.ignore();
            
            if (answer == problem.answer) {
                // Reduced base reward calculation
                int baseReward = OFFICE_WORK_REWARD / 2;  // Halved the base reward
                int reward = (baseReward * upgrade.level * problem.difficulty) / 2;  // Further reduced by difficulty scaling
                cout << "✅ Correct! You earned " << reward << " funds!\n";
                funds += reward;
                return true;
            } else {
                // Reduced consolation reward
                int consolationReward = OFFICE_WORK_REWARD / 4;  // Quarter of the base reward
                cout << "❌ Wrong! The correct answer was: " << problem.answer << "\n";
                cout << "You still get a small bonus for trying: " << consolationReward << " funds\n";
                funds += consolationReward;
                return false;
            }
        }
        
    public:
        Office(int& f, int& h, int& hu, int& l, int& lp, int& e, int& p, const string& cn, int up=1) 
            : funds(f), health(h), hunger(hu), level(l), levelPoints(lp), ecoPoints(e), pollutionLevel(p), cityName(cn) { upgrade.level = up; }
    
        void enter() {
            clearScreen();
            cout << OFFICE_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🏢 Welcome to your Office (Level " << upgrade.level << ")!\n";
                cout << "1. 💼 Work (Solve math problems to earn funds)\n";
                if (upgrade.canUpgrade()) cout << "2. Upgrade Office (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "3. 🚪 Exit Office\n";
                cout << "Choose an option: ";
    
                int choice;
                cin >> choice;
                cin.ignore();
    
                if (choice == 1) {
                    workSession();
                } else if (choice == 2 && upgrade.canUpgrade()) {
                    upgrade.upgrade(funds);
                } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                    cout << "🚶 Leaving Office...\n";
                    clearScreen();
                    return;
                } else {
                    cout << "Invalid option. Try again.\n";
                }
            }
        }

        int getUpgradeLevel() const { return upgrade.level; }

        void workSession() {
            cout << "💼 Working... (3 seconds)\n";
            this_thread::sleep_for(chrono::seconds(3));
            
            // Solve multiple problems based on office level
            int problemsToSolve = min(3, upgrade.level);
            int correctAnswers = 0;
            
            for (int i = 0; i < problemsToSolve; i++) {
                if (solveMathProblem()) {
                    correctAnswers++;
                }
                if (i < problemsToSolve - 1) {
                    cout << "\nPress Enter to continue to the next problem...";
                    cin.get();
                }
            }
            
            // Bonus for getting all problems correct
            if (correctAnswers == problemsToSolve) {
                int bonus = OFFICE_WORK_REWARD * upgrade.level;
                cout << "\n🎉 Perfect score! Bonus " << bonus << " funds!\n";
                funds += bonus;
            }
            
            // Reduce hunger after working
            int hungerReduction = 8 * upgrade.level;  // More hunger reduction at higher levels
            hunger = max(0, hunger - hungerReduction);
            cout << "\nYou solved " << correctAnswers << " out of " << problemsToSolve << " problems correctly!\n";
            cout << "🍽️ Working made you hungry! (-" << hungerReduction << " hunger)\n";
        }
    };

/**
 * @class Restaurant
 * @brief Represents the restaurant building
 * 
 * The restaurant provides meals that cost funds but give eco points.
 */
class Restaurant {
    private:
        int& funds;
        int& ecoPoints;
        int& hunger;
        int& health;
        int& level;
        int& levelPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
    public:
        Restaurant(int& f, int& e, int& h, int& he, int& l, int& lp, int& p, const string& cn, int up=1) 
            : funds(f), ecoPoints(e), hunger(h), health(he), level(l), levelPoints(lp), pollutionLevel(p), cityName(cn) { upgrade.level = up; }
    
        void enter() {
            clearScreen();
            cout << RESTAURANT_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🍴 Welcome to the Restaurant (Level " << upgrade.level << ")!\n";
                cout << "1. 🍞 Basic Meal (Costs " << BASIC_MEAL_COST << " funds, +" << (BASIC_MEAL_HUNGER * upgrade.level) << " hunger, -" << BASIC_MEAL_ECO_PENALTY << " ecoPoints)\n";
                cout << "2. 🍝 Intermediate Meal (Costs " << INTERMEDIATE_MEAL_COST << " funds, +" << (INTERMEDIATE_MEAL_HUNGER * upgrade.level) << " hunger)\n";
                cout << "3. 🍲 Premium Meal (Costs " << PREMIUM_MEAL_COST << " funds, +" << (PREMIUM_MEAL_ECO_POINTS * upgrade.level) << " ecoPoints, +" << (PREMIUM_MEAL_HUNGER * upgrade.level) << " hunger)\n";
                if (upgrade.canUpgrade()) cout << "4. Upgrade Restaurant (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "5. 🚪 Exit Restaurant\n";
                cout << "Choose an option: ";
    
                int choice;
                cin >> choice;
                cin.ignore();
    
                switch (choice) {
                    case 1:
                        if (funds >= BASIC_MEAL_COST) {
                            funds -= BASIC_MEAL_COST;
                            hunger = min(hunger + BASIC_MEAL_HUNGER * upgrade.level, MAX_HUNGER);
                            ecoPoints -= BASIC_MEAL_ECO_PENALTY;
                            cout << "🍽️ Eating basic meal...\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                        break;
                    case 2:
                        if (funds >= INTERMEDIATE_MEAL_COST) {
                            funds -= INTERMEDIATE_MEAL_COST;
                            hunger = min(hunger + INTERMEDIATE_MEAL_HUNGER * upgrade.level, MAX_HUNGER);
                            cout << "🍽️ Eating intermediate meal...\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                        break;
                    case 3:
                        if (funds >= PREMIUM_MEAL_COST) {
                            funds -= PREMIUM_MEAL_COST;
                            ecoPoints += PREMIUM_MEAL_ECO_POINTS * upgrade.level;
                            hunger = min(hunger + PREMIUM_MEAL_HUNGER * upgrade.level, MAX_HUNGER);
                            cout << "🍽️ Eating premium meal...\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                        break;
                    case 4:
                        if (upgrade.canUpgrade()) upgrade.upgrade(funds);
                        else cout << "Invalid option. Try again!\n";
                        break;
                    case 5:
                        cout << "🚶 Leaving Restaurant...\n";
                        clearScreen();
                        return;
                    default:
                        cout << "Invalid option. Try again!\n";
                }
            }
        }

        int getUpgradeLevel() const { return upgrade.level; }
    };

/**
 * @class School
 * @brief Represents the school building
 * 
 * The school allows players to study and gain eco points.
 */
class School {
    private:
        int& funds;
        int& ecoPoints;
        int& health;
        int& hunger;
        int& level;
        int& levelPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
        
        struct Riddle {
            string question;
            string answer;
            int ecoPoints;
            bool answered = false;  // Track if this riddle has been answered
        };
        
        vector<Riddle> riddles = {
            {"What grows in the ground but never dies?", "tree", 5},
            {"I am not alive, but I grow; I don't have lungs, but I need air; I don't have a mouth, but I can be killed. What am I?", "fire", 5},
            {"What has many keys but can't open a single lock?", "piano", 5},
            {"What gets wetter and wetter the more it dries?", "towel", 5},
            {"What has a head and a tail that will never meet?", "coin", 5},
            {"What has cities, but no houses; forests, but no trees; and rivers, but no water?", "map", 5},
            {"What can you catch but not throw?", "cold", 5},
            {"What has a neck but no head?", "bottle", 5},
            {"What has a face and two hands but no arms or legs?", "clock", 5},
            {"What has a thumb and four fingers but is not alive?", "glove", 5},
            {"What has a ring but no finger?", "telephone", 5},
            {"What has a bottom at the top?", "leg", 5},
            {"What has a head and a tail that will never meet?", "coin", 5},
            {"What has a neck but no head?", "bottle", 5},
            {"What has a face and two hands but no arms or legs?", "clock", 5},
            {"What has a thumb and four fingers but is not alive?", "glove", 5},
            {"What has a ring but no finger?", "telephone", 5},
            {"What has a bottom at the top?", "leg", 5},
            {"What has a head and a tail that will never meet?", "coin", 5},
            {"What has a neck but no head?", "bottle", 5}
        };
        
        Riddle getRandomRiddle() {
            static default_random_engine rng(static_cast<unsigned>(time(nullptr)));
            
            // Filter out answered riddles
            vector<Riddle> availableRiddles;
            for (const auto& riddle : riddles) {
                if (!riddle.answered) {
                    availableRiddles.push_back(riddle);
                }
            }
            
            // If all riddles have been answered, reset them
            if (availableRiddles.empty()) {
                cout << "\n🔄 All riddles have been answered! Resetting riddle pool...\n";
                for (auto& riddle : riddles) {
                    riddle.answered = false;
                }
                availableRiddles = riddles;
            }
            
            // Get random riddle from available ones
            uniform_int_distribution<size_t> dist(0, availableRiddles.size() - 1);
            return availableRiddles[dist(rng)];
        }
        
        bool askRiddle() {
            Riddle riddle = getRandomRiddle();
            cout << "\n📚 Here's your riddle:\n";
            cout << riddle.question << "\n";
            
            int attempts = 3;  // Total attempts allowed
            bool correct = false;
            
            while (attempts > 0 && !correct) {
                cout << "Your answer (Attempts left: " << attempts << "): ";
                string answer;
                getline(cin, answer);
                
                // Convert both to lowercase for case-insensitive comparison
                transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
                transform(riddle.answer.begin(), riddle.answer.end(), riddle.answer.begin(), ::tolower);
                
                if (answer == riddle.answer) {
                    cout << "✅ Correct! You earned " << (riddle.ecoPoints * upgrade.level) << " eco points!\n";
                    ecoPoints += riddle.ecoPoints * upgrade.level;
                    correct = true;
                } else {
                    attempts--;
                    if (attempts > 0) {
                        cout << "❌ Wrong! Try again.\n";
                    }
                }
            }
            
            if (!correct) {
                cout << "❌ Sorry, you're out of attempts. The correct answer was: " << riddle.answer << "\n";
            }
            
            // Mark this riddle as answered
            for (auto& r : riddles) {
                if (r.question == riddle.question) {
                    r.answered = true;
                    break;
                }
            }
            
            return correct;
        }
        
        void studySession() {
            cout << "📚 Studying... (4 seconds)\n";
            this_thread::sleep_for(chrono::seconds(4));
            
            // Ask up to 3 riddles based on school level
            int riddlesToAsk = min(3, upgrade.level);
            int correctAnswers = 0;
            
            for (int i = 0; i < riddlesToAsk; i++) {
                if (askRiddle()) {
                    correctAnswers++;
                }
                if (i < riddlesToAsk - 1) {
                    cout << "\nPress Enter to continue to the next riddle...";
                    cin.get();
                }
            }
            
            // Bonus eco points for getting all riddles correct
            if (correctAnswers == riddlesToAsk) {
                int bonusPoints = SCHOOL_STUDY_ECO_POINTS * upgrade.level;
                cout << "\n🎉 Perfect score! Bonus " << bonusPoints << " eco points!\n";
                ecoPoints += bonusPoints;
                updateLevel(level, levelPoints, ecoPoints, pollutionLevel);
            }
            
            // Reduce hunger after studying
            int hungerReduction = 5 * upgrade.level;  // More hunger reduction at higher levels
            hunger = max(0, hunger - hungerReduction);
            cout << "\nYou got " << correctAnswers << " out of " << riddlesToAsk << " riddles correct!\n";
            cout << "Total eco points gained: " << (correctAnswers * 5 * upgrade.level) << "\n";
            cout << "🍽️ Studying made you hungry! (-" << hungerReduction << " hunger)\n";
        }
        
    public:
        School(int& f, int& e, int& h, int& hu, int& l, int& lp, int& p, const string& cn, int up=1) 
            : funds(f), ecoPoints(e), health(h), hunger(hu), level(l), levelPoints(lp), pollutionLevel(p), cityName(cn) { upgrade.level = up; }
    
        void enter() {
            clearScreen();
            cout << SCHOOL_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🎓 Welcome to School (Level " << upgrade.level << ")!\n";
                cout << "1. 📚 Study and Solve Riddles (+" << (SCHOOL_STUDY_ECO_POINTS * upgrade.level) << " ecoPoints if correct)\n";
                if (upgrade.canUpgrade()) cout << "2. Upgrade School (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "3. 🚪 Exit School\n";
                cout << "Choose an option: ";
    
                int choice;
                cin >> choice;
                cin.ignore();
    
                if (choice == 1) {
                    studySession();
                } else if (choice == 2 && upgrade.canUpgrade()) {
                    upgrade.upgrade(funds);
                } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                    cout << "🚶 Leaving School...\n";
                    clearScreen();
                    return;
                } else {
                    cout << "Invalid option. Try again.\n";
                }
            }
        }

        int getUpgradeLevel() const { return upgrade.level; }
    };

/**
 * @class Bank
 * @brief Represents the bank building
 * 
 * The bank allows players to earn interest on their funds.
 */
class Bank {
    private:
        int& funds;
        int& health;
        int& hunger;
        int& level;
        int& levelPoints;
        int& ecoPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
    public:
        Bank(int& f, int& h, int& hu, int& l, int& lp, int& e, int& p, const string& cn, int up=1) 
            : funds(f), health(h), hunger(hu), level(l), levelPoints(lp), ecoPoints(e), pollutionLevel(p), cityName(cn) { upgrade.level = up; }
    
        void enter() {
            clearScreen();
            cout << BANK_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🏦 Welcome to the Bank (Level " << upgrade.level << ")!\n";
                cout << "1. 💰 Deposit and get interest (+" << (BANK_INTEREST_RATE * upgrade.level) << "% interest)\n";
                if (upgrade.canUpgrade()) cout << "2. Upgrade Bank (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "3. 🚪 Exit Bank\n";
                cout << "Choose an option: ";
    
                int choice;
                cin >> choice;
                cin.ignore();
    
                if (choice == 1) {
                    cout << "\n💰 Current Balance: " << funds << " funds\n";
                    cout << "Interest Rate: " << (BANK_INTEREST_RATE * upgrade.level) << "%\n";
                    cout << "Enter amount to deposit (0 to cancel): ";
                    int depositAmount;
                    cin >> depositAmount;
                    cin.ignore();
                    
                    if (depositAmount <= 0) {
                        cout << "Deposit cancelled.\n";
                        continue;
                    }
                    
                    if (depositAmount > funds) {
                        cout << "❌ Not enough funds!\n";
                        continue;
                    }
                    
                    cout << "💰 Depositing " << depositAmount << " funds and waiting for interest... (5 seconds)\n";
                    this_thread::sleep_for(chrono::seconds(5));
                    
                    int interest = (depositAmount * BANK_INTEREST_RATE * upgrade.level) / 100;
                    funds += interest;
                    cout << "You earned " << interest << " funds as interest! (Total funds: " << funds << ")\n";
                    
                    // Generate news about the deposit
                    string title = "Bank Deposit in " + cityName;
                    string content = "A citizen made a deposit of " + to_string(depositAmount) + " funds and earned " + to_string(interest) + " funds in interest.";
                    generateNews("💰 Economy", title, content);
                    
                } else if (choice == 2 && upgrade.canUpgrade()) {
                    upgrade.upgrade(funds);
                } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                    cout << "🚶 Leaving Bank...\n";
                    clearScreen();
                    return;
                } else {
                    cout << "Invalid option. Try again.\n";
                }
            }
        }

        int getUpgradeLevel() const { return upgrade.level; }
    };

/**
 * @class Casino
 * @brief Represents the casino building
 * 
 * The casino allows players to gamble their funds for a chance to win more.
 */
class Casino {
    private:
        int& funds;
        int& health;
        int& hunger;
        int& level;
        int& levelPoints;
        int& ecoPoints;
        int& pollutionLevel;
        BuildingUpgrade upgrade;
        const string& cityName;
    public:
        Casino(int& f, int& h, int& hu, int& l, int& lp, int& e, int& p, const string& cn, int up=1) 
            : funds(f), health(h), hunger(hu), level(l), levelPoints(lp), ecoPoints(e), pollutionLevel(p), cityName(cn) { upgrade.level = up; }
    
        void enter() {
            clearScreen();
            cout << CASINO_ART;
            while (true) {
                displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
                cout << "\n🎰 Welcome to the Casino (Level " << upgrade.level << ")!\n";
                cout << "1. 🎲 Simple Bet (Bet " << (CASINO_BET_AMOUNT * upgrade.level) << " funds)\n";
                cout << "2. 🎯 High Stakes (Bet " << (CASINO_BET_AMOUNT * 2 * upgrade.level) << " funds)\n";
                cout << "3. 🎮 Lucky Number (Bet " << (CASINO_BET_AMOUNT / 2 * upgrade.level) << " funds)\n";
                if (upgrade.canUpgrade()) cout << "4. Upgrade Casino (Cost: " << upgrade.upgradeCost() << " funds)\n";
                cout << "5. 🚪 Exit Casino\n";
                cout << "Choose an option: ";
    
                int choice;
                cin >> choice;
                cin.ignore();
    
                switch (choice) {
                    case 1:
                        gamble(CASINO_BET_AMOUNT * upgrade.level);
                        break;
                    case 2:
                        gamble(CASINO_BET_AMOUNT * 2 * upgrade.level);
                        break;
                    case 3:
                        gamble((CASINO_BET_AMOUNT / 2) * upgrade.level);
                        break;
                    case 4:
                        if (upgrade.canUpgrade()) upgrade.upgrade(funds);
                        else cout << "Invalid option. Try again!\n";
                        break;
                    case 5:
                        cout << "🚶 Leaving Casino...\n";
                        clearScreen();
                        return;
                    default:
                        cout << "Invalid option. Try again!\n";
                }
            }
        }

    private:
        void gamble(int bet) {
            if (funds >= bet) {
                funds -= bet;
                int roll = rand() % 2;
                cout << "🎲 Rolling... (3 seconds)\n";
                this_thread::sleep_for(chrono::seconds(3));
    
                if (roll == 1) {
                    cout << "🎉 You WON " << bet * 2 << " funds!\n";
                    funds += bet * 2;
                } else {
                    cout << "😢 You LOST the gamble.\n";
                }
                cout << "Current funds: " << funds << "\n";
            } else {
                cout << "❌ Not enough funds!\n";
            }
        }
    public:
        int getUpgradeLevel() const { return upgrade.level; }
    };

// --------- New Environment Class ---------
class Environment {
private:
    int& funds;
    int& ecoPoints;
    int& pollutionLevel;
    int& level;
    int& levelPoints;
    int& health;
    int& hunger;
    int treesPlanted;
    BuildingUpgrade upgrade;
    const string& cityName;
public:
    Environment(int& f, int& e, int& p, int& l, int& lp, int& h, int& hu, const string& cn, int up=1) 
        : funds(f), ecoPoints(e), pollutionLevel(p), level(l), levelPoints(lp), health(h), hunger(hu), cityName(cn), treesPlanted(0) { upgrade.level = up; }

    void enter() {
        clearScreen();
        cout << ENVIRONMENT_ART;
        while (true) {
            displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
            cout << "\n🌳 Welcome to the Environment Center (Level " << upgrade.level << ")!\n";
            cout << "1. 🌱 Plant a Tree (Costs " << (TREE_COST * upgrade.level) << " funds, +" << (TREE_ECO_POINTS * upgrade.level) << " ecoPoints, -" << (TREE_POLLUTION_REDUCTION * upgrade.level) << " pollution)\n";
            cout << "2. 📊 View Statistics\n";
            if (upgrade.canUpgrade()) cout << "3. Upgrade Environment Center (Cost: " << upgrade.upgradeCost() << " funds)\n";
            cout << "4. 🚪 Exit\n";
            cout << "Choose an option: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1:
                    plantTree();
                    break;
                case 2:
                    viewStatistics();
                    break;
                case 3:
                    if (upgrade.canUpgrade()) upgrade.upgrade(funds);
                    else cout << "Invalid option. Try again!\n";
                    break;
                case 4:
                    cout << "🚶 Leaving Environment Center...\n";
                    clearScreen();
                    return;
                default:
                    cout << "Invalid option. Try again!\n";
            }
        }
    }

    int getUpgradeLevel() const { return upgrade.level; }

private:
    void plantTree() {
        if (funds >= TREE_COST * upgrade.level) {
            funds -= TREE_COST * upgrade.level;
            if (pollutionLevel > 0) {
                pollutionLevel = max(0, pollutionLevel - (TREE_POLLUTION_REDUCTION * upgrade.level));
                cout << "🌱 Tree planted! Pollution reduced to " << pollutionLevel << "\n";
            } else {
                ecoPoints += 10 * upgrade.level;
                cout << "🌱 Tree planted! Pollution already at zero. Gained " << (10 * upgrade.level) << " eco points!\n";
            }
            ecoPoints += TREE_ECO_POINTS * upgrade.level;
            updateLevel(level, levelPoints, ecoPoints, pollutionLevel);
            treesPlanted++;
            
            // Generate news about tree planting
            string title = "New Trees Planted in " + cityName;
            string content = "The Environment Center has successfully planted new trees, contributing to the city's green initiative. Total trees planted: " + to_string(treesPlanted);
            generateNews("🌱 Environmental", title, content, true);
            
            cout << "Total trees: " << treesPlanted << "\n";
            achievements.treesPlanted++;
            achievements.checkAndUnlock();
            resetChallengeIfNeeded(dailyChallenge);
            if (!dailyChallenge.completed) {
                dailyChallenge.progress++;
                checkChallengeCompletion(dailyChallenge, funds, ecoPoints);
            }
        } else {
            cout << "❌ Not enough funds!\n";
        }
    }

    void viewStatistics() {
        cout << "\n📊 Environment Statistics:\n";
        cout << "🌳 Trees Planted: " << treesPlanted << "\n";
        cout << "🌱 Eco Points: " << ecoPoints << "\n";
        cout << "🌫️ Pollution Level: " << pollutionLevel << "\n";
        if (pollutionLevel < 0) {
            cout << "✨ Your city is pollution-free!\n";
        }
    }
};

// Add this function before the Summit class
vector<string> findFiles(const string& prefix) {
    vector<string> files;
    ifstream file;
    string filename;
    
    // Try to open a file with the prefix
    file.open(prefix + ".txt");
    if (file.is_open()) {
        file.close();
        files.push_back(prefix + ".txt");
    }
    
    // Try to open a file with the prefix and a number
    for (int i = 0; i <= 9; i++) {
        filename = prefix + to_string(i) + ".txt";
        file.open(filename);
        if (file.is_open()) {
            file.close();
            files.push_back(filename);
        }
    }
    
    return files;
}

// --------- Summit Class ---------
class Summit {
private:
    int& funds;
    int& ecoPoints;
    int& level;
    int& pollutionLevel;  // Add pollutionLevel reference
    string currentUser;
    string secondUser;
    int secondUserFunds;
    int& loanAmount;
    bool& hasLoan;
    string& lenderName;
    string& loanStartDate;

    void loadSecondUserData(const string& filename, int &fundsOut) {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string label;
                ss >> label;
                if (label == "funds:") {
                    ss >> fundsOut;
                }
            }
            file.close();
        }
    }

    void updateUserFunds(const string& filename, int newFunds) {
        // Read all lines, update funds, write back
        ifstream file(filename);
        vector<string> lines;
        string line;
        while (getline(file, line)) {
            if (line.find("funds:") == 0) {
                lines.push_back("funds: " + to_string(newFunds));
            } else {
                lines.push_back(line);
            }
        }
        file.close();
        ofstream out(filename);
        for (const auto& l : lines) out << l << endl;
        out.close();
    }

    void updateLoanStatus(const string& filename, bool hasLoan, int loanAmount, const string& lenderName) {
        // Read all lines, update or add loan info, write back
        ifstream file(filename);
        vector<string> lines;
        string line;
        bool foundHasLoan = false, foundLoanAmount = false, foundLender = false;
        while (getline(file, line)) {
            if (line.find("hasLoan:") == 0) {
                lines.push_back("hasLoan: " + to_string(hasLoan));
                foundHasLoan = true;
            } else if (line.find("loanAmount:") == 0) {
                lines.push_back("loanAmount: " + to_string(loanAmount));
                foundLoanAmount = true;
            } else if (line.find("lenderName:") == 0) {
                lines.push_back("lenderName: " + lenderName);
                foundLender = true;
            } else {
                lines.push_back(line);
            }
        }
        if (!foundHasLoan) lines.push_back("hasLoan: " + to_string(hasLoan));
        if (!foundLoanAmount) lines.push_back("loanAmount: " + to_string(loanAmount));
        if (!foundLender) lines.push_back("lenderName: " + lenderName);
        file.close();
        ofstream out(filename);
        for (const auto& l : lines) out << l << endl;
        out.close();
    }

    // In the Summit class, add this function before the enter() method
    private:
        void compareCityStats(const string& currentUser, const string& secondUser, int currentFunds, int secondFunds, 
                             int currentEco, int secondEco, int currentPollution, int secondPollution,
                             int currentLevel, int secondLevel) {
            cout << "\n🏙️ City Statistics Comparison\n";
            cout << "==========================\n\n";
            
            // Calculate the maximum length of city names for alignment
            int maxNameLength = max(currentUser.length(), secondUser.length());
            
            // Print header
            cout << setw(maxNameLength + 2) << "City" << " | " 
                 << setw(8) << "Funds" << " | " 
                 << setw(8) << "Eco" << " | " 
                 << setw(10) << "Pollution" << " | " 
                 << setw(6) << "Level" << endl;
            
            // Print separator
            cout << string(maxNameLength + 2, '-') << "-+-" 
                 << string(8, '-') << "-+-" 
                 << string(8, '-') << "-+-" 
                 << string(10, '-') << "-+-" 
                 << string(6, '-') << endl;
            
            // Print current user's stats
            cout << setw(maxNameLength + 2) << currentUser << " | " 
                 << setw(8) << currentFunds << " | " 
                 << setw(8) << currentEco << " | " 
                 << setw(10) << currentPollution << " | " 
                 << setw(6) << currentLevel << endl;
            
            // Print second user's stats
            cout << setw(maxNameLength + 2) << secondUser << " | " 
                 << setw(8) << secondFunds << " | " 
                 << setw(8) << secondEco << " | " 
                 << setw(10) << secondPollution << " | " 
                 << setw(6) << secondLevel << endl;
            
            cout << "\nPress Enter to continue...";
            cin.ignore();
        }

public:
    Summit(int& f, int& e, int& l, int& p, const string& user, bool& hLoan, int& lAmount, string& lName, string& lStartDate)
        : funds(f), ecoPoints(e), level(l), pollutionLevel(p), currentUser(user), loanAmount(lAmount), hasLoan(hLoan), lenderName(lName), loanStartDate(lStartDate) {}

    void enter() {
        if (level < SUMMIT_REQUIRED_LEVEL) {
            cout << "❌ You need to be at least level " << SUMMIT_REQUIRED_LEVEL << " to host a summit!\n";
            return;
        }
        if (funds < SUMMIT_FUNDS_COST) {
            cout << "❌ You need " << SUMMIT_FUNDS_COST << " funds to host a summit!\n";
            return;
        }
        if (hasLoan) {
            cout << "❌ You already have an active loan from " << lenderName << ". Repay it before taking another loan.\n";
        }
        cout << "\n🌍 Welcome to the Eco Summit!\n";
        cout << "Host: " << currentUser << "\n";
        cout << "Please enter the name of the second participant: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, secondUser);
        cout << "Please enter the PIN of " << secondUser << ": ";
        string secondPin;
        cin >> secondPin;
        cin.ignore();
        string secondUserFile = secondUser + "_" + secondPin + ".txt";
        ifstream checkFile(secondUserFile);
        if (!checkFile.good()) {
            cout << "❌ Invalid credentials for the second participant!\n";
            return;
        }
        // Load second user's data
        loadSecondUserData(secondUserFile, secondUserFunds);
        cout << "\n=== Eco Summit Begins ===\n";
        cout << "Participants: " << currentUser << " and " << secondUser << "\n\n";
        while (true) {
            cout << "\nSummit Activities:\n";
            cout << "1. Share Eco-Friendly Practices\n";
            cout << "2. Discuss Pollution Reduction Strategies\n";
            cout << "3. Plan Community Cleanup Events\n";
            cout << "4. Exchange Environmental Knowledge\n";
            cout << "5. Sign Eco-Partnership Agreement\n";
            cout << "6. Request Loan\n";
            cout << "7. Repay Loan\n";
            cout << "8. Compare City Statistics\n";
            cout << "9. End Summit\n";
            cout << "Choose an activity: ";
            int choice;
            cin >> choice;
            cin.ignore();

            if (choice == 1) {
                cout << "\n🌱 " << currentUser << " and " << secondUser << " share their eco-friendly practices:\n";
                cout << "- Using reusable shopping bags\n";
                cout << "- Implementing water conservation techniques\n";
                cout << "- Reducing plastic waste\n";
                cout << "- Using public transportation\n";
                cout << "- Composting organic waste\n";
                cout << "- Using energy-efficient appliances\n";
                cout << "- Participating in local environmental programs\n";
            }
            else if (choice == 2) {
                cout << "\n🌫️ " << currentUser << " and " << secondUser << " discuss pollution reduction:\n";
                cout << "- Implementing green energy solutions\n";
                cout << "- Reducing carbon footprint\n";
                cout << "- Promoting recycling programs\n";
                cout << "- Supporting clean air initiatives\n";
                cout << "- Developing waste management systems\n";
                cout << "- Creating green spaces in urban areas\n";
                cout << "- Implementing sustainable transportation\n";
            }
            else if (choice == 3) {
                cout << "\n🧹 " << currentUser << " and " << secondUser << " plan community cleanup:\n";
                cout << "- Organizing monthly cleanup drives\n";
                cout << "- Planting trees in public spaces\n";
                cout << "- Creating awareness campaigns\n";
                cout << "- Establishing recycling centers\n";
                cout << "- Setting up community gardens\n";
                cout << "- Organizing environmental workshops\n";
                cout << "- Implementing neighborhood composting\n";
            }
            else if (choice == 4) {
                cout << "\n📚 " << currentUser << " and " << secondUser << " exchange environmental knowledge:\n";
                cout << "- Sustainable farming techniques\n";
                cout << "- Renewable energy solutions\n";
                cout << "- Waste management strategies\n";
                cout << "- Environmental conservation methods\n";
                cout << "- Water conservation techniques\n";
                cout << "- Green building practices\n";
                cout << "- Wildlife protection strategies\n";
            }
            else if (choice == 5) {
                cout << "\n🤝 " << currentUser << " and " << secondUser << " sign an eco-partnership agreement:\n";
                cout << "- Committing to reduce carbon emissions\n";
                cout << "- Supporting each other's green initiatives\n";
                cout << "- Sharing resources and knowledge\n";
                cout << "- Working together for a sustainable future\n";
                cout << "- Regular environmental progress meetings\n";
                cout << "- Joint community projects\n";
                cout << "- Resource sharing program\n";
            }
            else if (choice == 6) {
                if (hasLoan) {
                    cout << "❌ You already have an active loan!\n";
                    continue;
                }
                cout << "\n💰 Loan Request\n";
                cout << secondUser << "'s available funds: " << secondUserFunds << "\n";
                cout << "Enter amount to request (0 to cancel): ";
                int requestAmount;
                cin >> requestAmount;
                cin.ignore();
                if (requestAmount <= 0) {
                    cout << "Loan request cancelled.\n";
                    continue;
                }
                if (requestAmount > secondUserFunds) {
                    cout << "❌ " << secondUser << " doesn't have enough funds!\n";
                    continue;
                }
                
                cout << "\nPlease ask " << secondUser << " to confirm the loan.\n";
                cout << "Enter " << secondUser << "'s PIN to confirm the loan: ";
                string confirmPin;
                cin >> confirmPin;
                cin.ignore();
                
                if (confirmPin != secondPin) {
                    cout << "❌ Invalid PIN! Loan request denied.\n";
                    continue;
                }
                
                cout << "\nLoan confirmed by " << secondUser << ".\n";
                loanAmount = requestAmount;
                hasLoan = true;
                lenderName = secondUser;
                funds += loanAmount;
                secondUserFunds -= loanAmount;
                
                // Update both users' files
                updateUserFunds(secondUserFile, secondUserFunds);
                string loanFile = currentUser + "_" + lenderName + ".txt";
                updateLoanStatus(loanFile, hasLoan, loanAmount, lenderName);
                updateLoanStatus(secondUserFile, false, 0, "");
                
                cout << "✅ Loan of " << loanAmount << " funds received from " << secondUser << "!\n";
                cout << "You will need to repay " << (loanAmount + (loanAmount * LOAN_INTEREST_RATE / 100)) << " funds.\n";
                achievements.loansRepaid++;
                achievements.checkAndUnlock();
                loanStartDate = getCurrentDateStrLoan();
                leaveMessage(secondUserFile, currentUser, "loan request");
            }
            else if (choice == 7) {
                if (!hasLoan) {
                    cout << "❌ You don't have any active loans!\n";
                    continue;
                }
                
                // Try to find the lender's file
                string lenderFile = lenderName + "_" + secondPin + ".txt";
                ifstream checkFile(lenderFile);
                if (!checkFile.good()) {
                    cout << "❌ Could not find lender's file! Please make sure the lender is present at the summit.\n";
                    continue;
                }
                
                int lenderFunds = 0;
                loadSecondUserData(lenderFile, lenderFunds);
                
                int repayAmount = loanAmount + (loanAmount * LOAN_INTEREST_RATE / 100);
                if (hasLoan && !loanStartDate.empty()) {
                    int days = daysBetween(loanStartDate, getCurrentDateStrLoan());
                    if (days > LOAN_DAYS_BEFORE_PENALTY) {
                        int penalty = (loanAmount * LOAN_LATE_PENALTY / 100);
                        repayAmount += penalty;
                        cout << "⚠️ Late repayment! Extra penalty applied: " << penalty << " funds.\n";
                    }
                }
                
                cout << "\n💰 Loan Repayment\n";
                cout << "Amount to repay: " << repayAmount << " funds\n";
                cout << "Your current funds: " << funds << "\n";
                cout << "Enter amount to repay (0 to cancel): ";
                int payment;
                cin >> payment;
                cin.ignore();
                
                if (payment <= 0) {
                    cout << "Repayment cancelled.\n";
                    continue;
                }
                if (payment > funds) {
                    cout << "❌ You don't have enough funds!\n";
                    continue;
                }
                
                // Process payment
                funds -= payment;
                lenderFunds += payment;
                
                // Update lender's file by reading all lines and updating funds
                vector<string> lines;
                string line;
                ifstream inFile(lenderFile);
                while (getline(inFile, line)) {
                    if (line.find("funds:") == 0) {
                        lines.push_back("funds: " + to_string(lenderFunds));
                    } else {
                        lines.push_back(line);
                    }
                }
                inFile.close();
                
                // Write back all lines with updated funds
                ofstream outFile(lenderFile);
                for (const auto& l : lines) {
                    outFile << l << endl;
                }
                outFile.close();
                
                if (payment < repayAmount) {
                    cout << "⚠️ Partial payment made. You still owe " << (repayAmount - payment) << " funds.\n";
                    loanAmount = repayAmount - payment;
                    updateLoanStatus(currentUser + "_" + lenderName + ".txt", hasLoan, loanAmount, lenderName);
                } else {
                    cout << "✅ Loan fully repaid!\n";
                    loanAmount = 0;
                    hasLoan = false;
                    lenderName = "";
                    updateLoanStatus(currentUser + "_" + lenderName + ".txt", hasLoan, loanAmount, lenderName);
                    loanStartDate = "";
                }
                
                achievements.loansRepaid++;
                achievements.checkAndUnlock();
                
                // Notify lender of repayment
                notifyLenderOfRepayment(lenderFile, currentUser);
                
                cout << "✅ Payment of " << payment << " funds has been transferred to " << lenderName << ".\n";
                cout << "Lender's new balance: " << lenderFunds << " funds.\n";
            }
            else if (choice == 8) {
                // Load second user's data
                int secondEco = 0;
                int secondPollution = 0;
                int secondLevel = 0;
                
                ifstream secondFile(secondUserFile);
                string line;
                while (getline(secondFile, line)) {
                    stringstream ss(line);
                    string label;
                    ss >> label;
                    if (label == "ecopoints:") ss >> secondEco;
                    else if (label == "pollutionlevel:") ss >> secondPollution;
                    else if (label == "level:") ss >> secondLevel;
                }
                secondFile.close();
                
                compareCityStats(currentUser, secondUser, funds, secondUserFunds, 
                               ecoPoints, secondEco, pollutionLevel, secondPollution,
                               level, secondLevel);
            }
            else if (choice == 9) {
                cout << "\n✅ Summit completed successfully!\n";
                cout << "💰 Cost: " << SUMMIT_FUNDS_COST << " funds\n";
                cout << "🌱 Gained: " << SUMMIT_ECO_POINTS << " eco points\n";
                funds -= SUMMIT_FUNDS_COST;
                ecoPoints += SUMMIT_ECO_POINTS;
                achievements.summitsHosted++;
                achievements.checkAndUnlock();
                resetChallengeIfNeeded(weeklyChallenge);
                if (!weeklyChallenge.completed) {
                    weeklyChallenge.progress++;
                    checkChallengeCompletion(weeklyChallenge, funds, ecoPoints);
                }
                leaveMessage(secondUserFile, currentUser, "eco summit");
                clearScreen();
                return;
            }
            else {
                cout << "Invalid choice!\n";
            }
        }
    }
};

// --------- New Building: Recycling Center ---------
class RecyclingCenter {
private:
    int& funds;
    int& pollutionLevel;
    int& level;
    int& levelPoints;
    int& ecoPoints;
    int& health;
    int& hunger;
    BuildingUpgrade upgrade;
    const string& cityName;
public:
    RecyclingCenter(int& f, int& p, int& l, int& lp, int& e, int& h, int& hu, const string& cn, int up=1) 
        : funds(f), pollutionLevel(p), level(l), levelPoints(lp), ecoPoints(e), health(h), hunger(hu), cityName(cn) { upgrade.level = up; }
    void enter() {
        clearScreen();
        cout << RECYCLING_ART;
        while (true) {
            displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
            cout << "\n♻️ Welcome to the Recycling Center (Level " << upgrade.level << ")!\n";
            cout << "1. Recycle Waste (Reduces pollution by " << (10 * upgrade.level) << ")\n";
            if (upgrade.canUpgrade()) cout << "2. Upgrade Center (Cost: " << upgrade.upgradeCost() << " funds)\n";
            cout << "3. Exit\nChoose an option: ";
            int choice;
            cin >> choice;
            cin.ignore();
            if (choice == 1) {
                int reduction = 10 * upgrade.level;
                int oldPollution = pollutionLevel;
                if (pollutionLevel > 0) {
                    pollutionLevel = max(0, pollutionLevel - reduction);
                    cout << "You recycled waste! Pollution reduced from " << oldPollution << " to " << pollutionLevel << ".\n";
                } else {
                    ecoPoints += 10 * upgrade.level;
                    cout << "You recycled waste! Pollution already at zero. Gained " << (10 * upgrade.level) << " eco points!\n";
                }
                
                // Generate news about recycling
                string title = "Recycling Initiative Success in " + cityName;
                string content = "The Recycling Center has successfully processed waste, reducing pollution from " + to_string(oldPollution) + " to " + to_string(pollutionLevel);
                generateNews("🌱 Environmental", title, content);
                
                this_thread::sleep_for(chrono::seconds(2));
                clearScreen();
            } else if (choice == 2 && upgrade.canUpgrade()) {
                upgrade.upgrade(funds);
            } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                cout << "Leaving Recycling Center...\n";
                clearScreen();
                return;
            } else {
                cout << "Invalid option. Try again!\n";
            }
        }
    }

    int getUpgradeLevel() const { return upgrade.level; }
};

// --------- New Building: Community Garden ---------
class CommunityGarden {
private:
    int& funds;
    int& ecoPoints;
    int& health;
    int& level;
    int& levelPoints;
    int& pollutionLevel;
    int& hunger;
    BuildingUpgrade upgrade;
    const string& cityName;
public:
    CommunityGarden(int& f, int& e, int& h, int& l, int& lp, int& p, int& hu, const string& cn, int up=1) 
        : funds(f), ecoPoints(e), health(h), level(l), levelPoints(lp), pollutionLevel(p), hunger(hu), cityName(cn) { upgrade.level = up; }
    void enter() {
        clearScreen();
        cout << GARDEN_ART;
        while (true) {
            displayStatus(funds, health, hunger, level, levelPoints, ecoPoints, pollutionLevel, cityName);
            cout << "\n🌻 Welcome to the Community Garden (Level " << upgrade.level << ")!\n";
            cout << "1. Volunteer Gardening (+" << (5 * upgrade.level) << " eco points, +" << (3 * upgrade.level) << " health)\n";
            if (upgrade.canUpgrade()) cout << "2. Upgrade Garden (Cost: " << upgrade.upgradeCost() << " funds)\n";
            cout << "3. Exit\nChoose an option: ";
            int choice;
            cin >> choice;
            cin.ignore();
            if (choice == 1) {
                ecoPoints += 5 * upgrade.level;
                health = min(health + 3 * upgrade.level, MAX_HEALTH);
                cout << "You volunteered in the garden! Eco points and health increased.\n";
            } else if (choice == 2 && upgrade.canUpgrade()) {
                upgrade.upgrade(funds);
            } else if ((choice == 2 && !upgrade.canUpgrade()) || choice == 3) {
                cout << "Leaving Community Garden...\n";
                clearScreen();
                return;
            } else {
                cout << "Invalid option. Try again!\n";
            }
        }
    }

    int getUpgradeLevel() const { return upgrade.level; }
};

// --------- Environmental News Feed ---------
string getCurrentTimeStr() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&t));
    return string(buf);
}

void generateNews(const string& category, const string& title, const string& content, bool isImportant) {
    NewsItem news;
    news.title = title;
    news.content = content;
    news.category = category;
    news.timestamp = getCurrentTimeStr();
    news.isImportant = isImportant;
    newsFeed.push_back(news);
    
    // Create file if it doesn't exist and append news
    ofstream out("news_feed.txt", ios::app);
    if (!out.is_open()) {
        cerr << "Error: Could not open news_feed.txt for writing\n";
        return;
    }
    out << "[" << news.timestamp << "] " << news.category << " - " << news.title << "\n";
    out << news.content << "\n\n";
    out.close();
    
    // Print the news to console as well
    cout << "\n📰 Breaking News!\n";
    cout << "[" << news.timestamp << "] " << news.category << " - " << news.title << "\n";
    cout << content << "\n\n";
    this_thread::sleep_for(chrono::seconds(2));
}

void generateRandomNews(int level, int ecoPoints, int pollutionLevel, const string& cityName) {
    static default_random_engine rng(static_cast<unsigned>(time(nullptr)));
    uniform_int_distribution<int> dist(1, 100);
    
    if (dist(rng) <= 20) { // 20% chance to generate random news
        vector<string> categories = {"🌱 Environmental", "🏙️ City Development", "💰 Economy", "🌍 Global"};
        vector<string> environmentalNews = {
            "Local Wildlife Returns to City Parks",
            "New Species of Birds Spotted in Urban Areas",
            "Community Garden Yields Record Harvest",
            "Solar Panel Installation Reaches New Heights",
            "Air Quality Shows Significant Improvement"
        };
        vector<string> cityNews = {
            "New Eco-Friendly Buildings Rise in Downtown",
            "Public Transport System Expands",
            "City Announces Green Energy Initiative",
            "Urban Forest Project Takes Root",
            "Smart City Technology Implementation Begins"
        };
        vector<string> economyNews = {
            "Green Jobs Market Booms",
            "Eco-Tourism Revenue Increases",
            "Sustainable Business Awards Announced",
            "Local Farmers Market Success",
            "Renewable Energy Investment Grows"
        };
        vector<string> globalNews = {
            "International Climate Summit Results",
            "Global Green Technology Exchange",
            "Worldwide Pollution Reduction Goals Met",
            "International Eco-City Partnership",
            "Global Environmental Treaty Signed"
        };
        
        string category = categories[dist(rng) % categories.size()];
        string title;
        string content;
        
        if (category == "🌱 Environmental") {
            title = environmentalNews[dist(rng) % environmentalNews.size()];
            content = "Environmental conditions in " + cityName + " continue to improve. Local initiatives are showing positive results.";
        } else if (category == "🏙️ City Development") {
            title = cityNews[dist(rng) % cityNews.size()];
            content = "City development projects are progressing well. New infrastructure is being built with sustainability in mind.";
        } else if (category == "💰 Economy") {
            title = economyNews[dist(rng) % economyNews.size()];
            content = "The local economy is thriving with a focus on sustainable development and green initiatives.";
        } else {
            title = globalNews[dist(rng) % globalNews.size()];
            content = "Global environmental efforts are making progress. International cooperation is key to success.";
        }
        
        generateNews(category, title, content);
    }
}

void showNewsFeed() {
    clearScreen();
    const int MAX_NEWS_TO_SHOW = 5;  // Show only 5 most recent news items per page
    
    ifstream in("news_feed.txt");
    if (!in.is_open()) {
        cout << "No news to display yet. Take actions in the city to generate news!\n\n";
        cout << "Press Enter to continue...";
        cin.ignore();
        clearScreen();
        return;
    }
    
    // Read all news items into a vector
    vector<string> newsItems;
    string line;
    while (getline(in, line)) {
        if (!line.empty()) {
            newsItems.push_back(line);
        }
    }
    in.close();
    
    size_t totalItems = newsItems.size();
    if (totalItems == 0) {
        cout << "No news to display yet. Take actions in the city to generate news!\n";
        cout << "\nPress Enter to continue...";
        cin.ignore();
        clearScreen();
        return;
    }

    size_t currentPage = 0;
    size_t totalPages = (totalItems + MAX_NEWS_TO_SHOW - 1) / MAX_NEWS_TO_SHOW;
    
    while (true) {
        clearScreen();
        cout << "\n📰 Environmental News Feed\n";
        cout << "========================\n\n";
        
        // Calculate start and end indices for current page
        size_t startIdx = totalItems - (currentPage + 1) * MAX_NEWS_TO_SHOW;
        size_t endIdx = totalItems - currentPage * MAX_NEWS_TO_SHOW;
        
        // Adjust startIdx if it would be negative
        if (startIdx > totalItems) startIdx = 0;
        
        // Display news items for current page
        for (size_t i = startIdx; i < endIdx; i++) {
            cout << newsItems[i] << "\n";
        }
        
        cout << "\nPage " << (currentPage + 1) << " of " << totalPages;
        cout << " (Showing " << (endIdx - startIdx) << " of " << totalItems << " news items)\n";
        cout << "\nOptions:\n";
        cout << "1. Next Page\n";
        cout << "2. Previous Page\n";
        cout << "3. Exit News Feed\n";
        cout << "Choose an option: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: // Next Page
                if (currentPage < totalPages - 1) {
                    currentPage++;
                } else {
                    cout << "\nYou're already on the last page!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                }
                break;
            case 2: // Previous Page
                if (currentPage > 0) {
                    currentPage--;
                } else {
                    cout << "\nYou're already on the first page!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                }
                break;
            case 3: // Exit
                clearScreen();
                return;
            default:
                cout << "\nInvalid option! Press Enter to continue...";
                cin.get();
                break;
        }
    }
}

struct RandomEvent {
    string description;
    function<void(int&, int&, int&, int&)> effect;  // Modified to take references to game state variables
};

vector<RandomEvent> randomEvents = {
    {"🌧️ Heavy Rainfall: Pollution decreases!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Pollution level decreased by 10!\n"; 
         pollutionlevel = max(0, pollutionlevel - 10); 
     }},
    {"💸 Eco Grant: You receive a government grant!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Funds increased by 200!\n"; 
         funds += 200; 
     }},
    {"🦠 Disease Outbreak: Health drops!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Health decreased by 20!\n"; 
         health = max(0, health - 20); 
     }},
    {"🌪️ Pollution Spike: Pollution increases!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Pollution level increased by 15!\n"; 
         pollutionlevel += 15; 
     }},
    {"🌱 Community Cleanup: Eco points increased!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Eco points increased by 30!\n"; 
         ecopoints += 30; 
     }},
    {"💰 Unexpected Expense: Funds decreased!", 
     [](int& funds, int& health, int& ecopoints, int& pollutionlevel) { 
         cout << "Funds decreased by 100!\n"; 
         funds = max(0, funds - 100); 
     }}
};

void triggerRandomEvent(int& funds, int& health, int& ecopoints, int& pollutionlevel) {
    static default_random_engine rng(static_cast<unsigned>(time(nullptr)));
    uniform_int_distribution<int> dist(1, 10); // 10% chance per menu loop
    if (dist(rng) == 1) {
        uniform_int_distribution<size_t> eventDist(0, randomEvents.size() - 1);
        size_t idx = eventDist(rng);
        cout << "\n=== Random Event! ===\n";
        cout << randomEvents[idx].description << "\n";
        randomEvents[idx].effect(funds, health, ecopoints, pollutionlevel);
        cout << "====================\n";
    }
}

// --------- Function Implementations ---------
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void displayStatusBar(int value, int max, const string& label, const string& color) {
    const int barWidth = 20;
    float ratio = static_cast<float>(value) / max;
    int filled = static_cast<int>(barWidth * ratio);
    cout << color << label << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < filled) cout << "█";
        else cout << "░";
    }
    cout << "] " << value << "/" << max << "\033[0m\n";
}

void displayStatus(int funds, int health, int hunger, int level, int levelPoints, int ecoPoints, int pollutionLevel, const string& cityName) {
    cout << "\n🏙️ City: " << cityName << "\n";
    cout << "=== Status ===\n";
    cout << "💰 Funds: " << funds << "\n";
    displayStatusBar(health, MAX_HEALTH, "❤️ Health", "\033[1;31m");
    displayStatusBar(hunger, MAX_HUNGER, "🍽️ Hunger", "\033[1;33m");
    cout << "📊 Level: " << level << " (Points: " << levelPoints << "/" << LEVEL_UP_THRESHOLD << ")\n";
    cout << "🌱 Eco Points: " << ecoPoints << "\n";
    displayStatusBar(pollutionLevel, 100, "🌫️ Pollution", "\033[1;37m");
    cout << "==============\n";
}

void updateLevel(int& level, int& levelPoints, int ecoPoints, int pollutionLevel) {
    // Calculate total points considering eco points and pollution penalty
    int totalPoints = ecoPoints - (pollutionLevel * POLLUTION_PENALTY);
    if (totalPoints < 0) totalPoints = 0;
    
    // Calculate level points (remainder after dividing by threshold)
    levelPoints = totalPoints % LEVEL_UP_THRESHOLD;
    
    // Calculate new level
    int newLevel = (totalPoints / LEVEL_UP_THRESHOLD) + 1;
    
    // Check if level up occurred
    if (newLevel > level) {
        level = newLevel;
        cout << "\n🎉 Level Up! You are now level " << level << "!\n";
        cout << "You need " << ((level * LEVEL_UP_THRESHOLD) - totalPoints) << " more points for the next level.\n";
    }
}

void updateHungerAndHealth(int& health, int& hunger, time_t& lastUpdateTime) {
    time_t currentTime = time(nullptr);
    int timeDiff = static_cast<int>(difftime(currentTime, lastUpdateTime));
    
    if (timeDiff >= 60) { // Update every minute
        // Decrease hunger
        hunger = max(0, hunger - (HUNGER_DECREASE_RATE * (timeDiff / 60)));
        
        // Always decrease health by 1 per minute
        health = max(0, health - (HEALTH_DECREASE_RATE * (timeDiff / 60)));
        
        // Additional health decrease if hunger is low
        if (hunger < HUNGER_THRESHOLD) {
            health = max(0, health - (HEALTH_DECREASE_RATE_LOW_HUNGER * (timeDiff / 60)));
            cout << "\n⚠️ Warning: Low hunger! Health is decreasing rapidly.\n";
            cout << "Current Health: " << health << "/" << MAX_HEALTH << "\n";
            cout << "Current Hunger: " << hunger << "/" << MAX_HUNGER << "\n";
        }
        
        lastUpdateTime = currentTime;
    }
}

void transport_delay(int vehicle, int& ecoPoints, int& pollutionLevel) {
    switch (vehicle) {
        case 0: // Walking
            cout << "🚶 Walking...\n";
            this_thread::sleep_for(chrono::seconds(WALK_DELAY));
            break;
        case 1: // Cycling
            cout << "🚲 Cycling...\n";
            this_thread::sleep_for(chrono::seconds(CYCLE_DELAY));
            ecoPoints += 5;
            break;
        case 2: // Regular Car
            cout << "🚗 Driving...\n";
            this_thread::sleep_for(chrono::seconds(CAR_DELAY));
            ecoPoints -= CAR_ECO_PENALTY;
            if (pollutionLevel > 0) {
                pollutionLevel = min(100, pollutionLevel + CAR_POLLUTION_INCREASE);
            } else {
                ecoPoints -= 10; // Extra penalty for using car when pollution is zero
            }
            break;
        case 3: // Electric Car
            cout << "🚗 Driving (Electric)...\n";
            this_thread::sleep_for(chrono::seconds(CAR_DELAY));
            ecoPoints += ELECTRIC_CAR_ECO_BONUS;
            break;
    }
}

void update_funds_periodically(int& funds, time_t& lastUpdateTime) {
    time_t currentTime = time(nullptr);
    int timeDiff = static_cast<int>(difftime(currentTime, lastUpdateTime));
    
    if (timeDiff >= PERIODIC_FUNDS_INTERVAL) {
        funds += PERIODIC_FUNDS_REWARD * (timeDiff / PERIODIC_FUNDS_INTERVAL);
        lastUpdateTime = currentTime;
    }
}

void offline_bonus(int& funds, time_t& lastSaveTime) {
    time_t currentTime = time(nullptr);
    int days = static_cast<int>(difftime(currentTime, lastSaveTime) / (24 * 3600));
    
    if (days > 0) {
        funds += OFFLINE_BONUS_PER_DAY * days;
        cout << "💰 Offline bonus: " << (OFFLINE_BONUS_PER_DAY * days) << " funds\n";
    }
}

bool load_game(string filename, int& level, int& ecopoints, int& funds, int& health, int& hunger,
    int &houseUp, int &hospitalUp, int &officeUp, int &restaurantUp, int &schoolUp, int &bankUp, int &casinoUp, int &envUp, int &recUp, int &gardenUp,
    int& vehicle, int& pollutionlevel, time_t& lastSaveTime, int& levelPoints,
    bool &hasLoan, int &loanAmount, string &lenderName, string &loanStartDate, string &cityName, int& daysWithZeroPollution) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string label;
        ss >> label;
        if (label == "level:") ss >> level;
        else if (label == "ecopoints:") ss >> ecopoints;
        else if (label == "funds:") ss >> funds;
        else if (label == "health:") ss >> health;
        else if (label == "hunger:") ss >> hunger;
        else if (label == "houseUp:") ss >> houseUp;
        else if (label == "hospitalUp:") ss >> hospitalUp;
        else if (label == "officeUp:") ss >> officeUp;
        else if (label == "restaurantUp:") ss >> restaurantUp;
        else if (label == "schoolUp:") ss >> schoolUp;
        else if (label == "bankUp:") ss >> bankUp;
        else if (label == "casinoUp:") ss >> casinoUp;
        else if (label == "envUp:") ss >> envUp;
        else if (label == "recUp:") ss >> recUp;
        else if (label == "gardenUp:") ss >> gardenUp;
        else if (label == "vehicle:") ss >> vehicle;
        else if (label == "pollutionlevel:") ss >> pollutionlevel;
        else if (label == "lastSaveTime:") ss >> lastSaveTime;
        else if (label == "levelPoints:") ss >> levelPoints;
        else if (label == "hasLoan:") ss >> hasLoan;
        else if (label == "loanAmount:") ss >> loanAmount;
        else if (label == "lenderName:") ss >> lenderName;
        else if (label == "loanStartDate:") ss >> loanStartDate;
        else if (label == "cityName:") getline(ss, cityName), cityName = cityName.substr(1);
        else if (label == "hasBicycle:") ss >> hasBicycle;
        else if (label == "hasCar:") ss >> hasCar;
        else if (label == "hasElectricCar:") ss >> hasElectricCar;
        else if (label == "daysWithZeroPollution:") ss >> daysWithZeroPollution;
    }
    file.close();
    return true;
}

void save_game(string filename, int level, int ecopoints, int funds, int health, int hunger,
    int houseUp, int hospitalUp, int officeUp, int restaurantUp, int schoolUp, int bankUp, int casinoUp, int envUp, int recUp, int gardenUp,
    int vehicle, int pollutionlevel, time_t lastSaveTime, int levelPoints, bool hasLoan, int loanAmount, string lenderName, string loanStartDate, string cityName, int daysWithZeroPollution) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "level: " << level << endl;
        file << "ecopoints: " << ecopoints << endl;
        file << "funds: " << funds << endl;
        file << "health: " << health << endl;  // Ensure health is saved
        file << "hunger: " << hunger << endl;  // Ensure hunger is saved
        file << "houseUp: " << houseUp << endl;
        file << "hospitalUp: " << hospitalUp << endl;
        file << "officeUp: " << officeUp << endl;
        file << "restaurantUp: " << restaurantUp << endl;
        file << "schoolUp: " << schoolUp << endl;
        file << "bankUp: " << bankUp << endl;
        file << "casinoUp: " << casinoUp << endl;
        file << "envUp: " << envUp << endl;
        file << "recUp: " << recUp << endl;
        file << "gardenUp: " << gardenUp << endl;
        file << "vehicle: " << vehicle << endl;
        file << "pollutionlevel: " << pollutionlevel << endl;
        file << "lastSaveTime: " << lastSaveTime << endl;
        file << "levelPoints: " << levelPoints << endl;
        file << "hasLoan: " << hasLoan << endl;
        file << "loanAmount: " << loanAmount << endl;
        file << "lenderName: " << lenderName << endl;
        file << "loanStartDate: " << loanStartDate << endl;
        file << "cityName: " << cityName << endl;
        file << "hasBicycle: " << hasBicycle << endl;
        file << "hasCar: " << hasCar << endl;
        file << "hasElectricCar: " << hasElectricCar << endl;
        file << "daysWithZeroPollution: " << daysWithZeroPollution << endl;
        file.close();
    }
}

// ------------ Main Menu ------------
void menu(int &funds, int &vehicle, int &level, time_t &lastUpdateTime, int &ecopoints, int &health, int &hunger, int &levelPoints, int &pollutionLevel, const string& currentUser, bool &hasLoan, int &loanAmount, string &lenderName, string &loanStartDate,
    int &houseUp, int &hospitalUp, int &officeUp, int &restaurantUp, int &schoolUp, int &bankUp, int &casinoUp, int &envUp, int &recUp, int &gardenUp, const string& cityName, int &daysWithZeroPollution)
{
    time_t lastHealthUpdate = time(nullptr);
    int initialFunds = funds;
    static int lastMenuVisit = 0;  // Track menu visits instead of real time
    
    while (true)
    {
        // Check for game over conditions
        if (health <= 0 || hunger <= 0) {
            clearScreen();
            cout << "\n❌ GAME OVER ❌\n";
            cout << "Your health or hunger reached zero!\n";
            cout << "Final Stats:\n";
            cout << "Level: " << level << "\n";
            cout << "Eco Points: " << ecopoints << "\n";
            cout << "Funds: " << funds << "\n";
            return;
        }
        
        // Check for win condition (every 10 menu visits = 1 in-game day)
        lastMenuVisit++;
        if (lastMenuVisit >= 10) {  // One in-game day has passed
            if (pollutionLevel == 0) {
                daysWithZeroPollution++;
                if (level >= WIN_REQUIRED_LEVEL && daysWithZeroPollution >= WIN_REQUIRED_DAYS) {
                    clearScreen();
                    cout << "\n🎉 VICTORY! 🎉\n";
                    cout << "You have achieved the ultimate eco-city!\n";
                    cout << "Final Stats:\n";
                    cout << "Level: " << level << "\n";
                    cout << "Eco Points: " << ecopoints << "\n";
                    cout << "Funds: " << funds << "\n";
                    cout << "Days with Zero Pollution: " << daysWithZeroPollution << "\n";
                    return;
                }
            } else {
                daysWithZeroPollution = 0;
            }
            lastMenuVisit = 0;  // Reset for next in-game day
        }
        
        triggerRandomEvent(funds, health, ecopoints, pollutionLevel);
        update_funds_periodically(funds, lastUpdateTime);
        updateHungerAndHealth(health, hunger, lastHealthUpdate);
        updateLevel(level, levelPoints, ecopoints, pollutionLevel);
        
        // Generate random news periodically
        generateRandomNews(level, ecopoints, pollutionLevel, cityName);
        
        displayStatus(funds, health, hunger, level, levelPoints, ecopoints, pollutionLevel, cityName);

        cout << "\n--- Eco City Menu ---\n";
        cout << "1. View Instructions\n";
        cout << "2. View Achievements\n";
        cout << "3. View Challenges\n";
        cout << "4. Change Transport Mode\n";
        cout << "5. Go to House\n";
        cout << "6. Go to Hospital\n";
        cout << "7. Go to Office\n";
        cout << "8. Go to Restaurant\n";
        cout << "9. Go to School\n";
        cout << "10. Go to Bank\n";
        cout << "11. Go to Casino\n";
        cout << "12. Environment Center\n";
        cout << "13. Host Eco Summit\n";
        cout << "14. Recycling Center\n";
        cout << "15. Community Garden\n";
        cout << "16. View News Feed\n";
        cout << "0. Save and Exit\n";
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;

        int netFunds = funds - initialFunds;

        switch (choice)
        {
            case 1:
                displayInstructions("eco_city_instructions.txt");
                break;
            case 2:
                achievements.display();
                break;
            case 3:
                showChallenges();
                break;
            case 4:
                cout << "\n=== Transport Options ===\n";
                cout << "Current vehicle: ";
                switch(vehicle) {
                    case 0: cout << "Walking"; break;
                    case 1: cout << "Bicycle"; break;
                    case 2: cout << "Regular Car"; break;
                    case 3: cout << "Electric Car"; break;
                }
                cout << "\n\nAvailable vehicles:\n";
                cout << "0. Walk (Free)\n";
                
                // Show bicycle options based on level and ownership
                if (level >= CYCLE_REQUIRED_LEVEL) {
                    if (hasBicycle) {
                        cout << "1. Cycle (Owned) ✓\n";
                    } else {
                        cout << "1. Buy Cycle (" << CYCLE_COST << " funds)\n";
                    }
                } else {
                    cout << "1. Cycle (Requires level " << CYCLE_REQUIRED_LEVEL << ")\n";
                }
                
                // Show regular car options based on level and ownership
                if (level >= CAR_REQUIRED_LEVEL) {
                    if (hasCar) {
                        cout << "2. Regular Car (Owned) ✓\n";
                    } else {
                        cout << "2. Buy Regular Car (" << CAR_COST << " funds)\n";
                    }
                } else {
                    cout << "2. Regular Car (Requires level " << CAR_REQUIRED_LEVEL << ")\n";
                }
                
                // Show electric car options based on level and ownership
                if (level >= ELECTRIC_CAR_REQUIRED_LEVEL) {
                    if (hasElectricCar) {
                        cout << "3. Electric Car (Owned) ✓\n";
                    } else {
                        cout << "3. Buy Electric Car (" << ELECTRIC_CAR_COST << " funds)\n";
                    }
                } else {
                    cout << "3. Electric Car (Requires level " << ELECTRIC_CAR_REQUIRED_LEVEL << ")\n";
                }
                
                cout << "\nChoose an option: ";
                cin >> choice;
                
                if (choice == 1) {
                    if (level < CYCLE_REQUIRED_LEVEL) {
                        cout << "❌ You need to be level " << CYCLE_REQUIRED_LEVEL << " to buy a bicycle!\n";
                    } else if (!hasBicycle) {
                        if (funds >= CYCLE_COST) {
                            funds -= CYCLE_COST;
                            hasBicycle = true;
                            vehicle = 1;
                            cout << "✅ Bicycle purchased! You can now cycle.\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                    } else {
                        vehicle = 1;
                        cout << "✅ Transport mode changed to cycling!\n";
                    }
                } else if (choice == 2) {
                    if (level < CAR_REQUIRED_LEVEL) {
                        cout << "❌ You need to be level " << CAR_REQUIRED_LEVEL << " to buy a regular car!\n";
                    } else if (!hasCar) {
                        if (funds >= CAR_COST) {
                            funds -= CAR_COST;
                            hasCar = true;
                            vehicle = 2;
                            cout << "✅ Regular car purchased! You can now drive.\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                    } else {
                        vehicle = 2;
                        cout << "✅ Transport mode changed to regular car!\n";
                    }
                } else if (choice == 3) {
                    if (level < ELECTRIC_CAR_REQUIRED_LEVEL) {
                        cout << "❌ You need to be level " << ELECTRIC_CAR_REQUIRED_LEVEL << " to buy an electric car!\n";
                    } else if (!hasElectricCar) {
                        if (funds >= ELECTRIC_CAR_COST) {
                            funds -= ELECTRIC_CAR_COST;
                            hasElectricCar = true;
                            vehicle = 3;
                            cout << "✅ Electric car purchased! You can now drive eco-friendly.\n";
                        } else {
                            cout << "❌ Not enough funds!\n";
                        }
                    } else {
                        vehicle = 3;
                        cout << "✅ Transport mode changed to electric car!\n";
                    }
                } else if (choice == 0) {
                    vehicle = 0;
                    cout << "✅ Transport mode changed to walking!\n";
                } else {
                    cout << "Invalid choice!\n";
                }
                break;
            case 5:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { House h(funds, ecopoints, health, hunger, level, levelPoints, pollutionLevel, cityName, houseUp); h.enter(); houseUp = h.getUpgradeLevel(); }
                break;
            case 6:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { Hospital h(funds, health, hunger, level, levelPoints, ecopoints, pollutionLevel, cityName, hospitalUp); h.enter(); hospitalUp = h.getUpgradeLevel(); }
                break;
            case 7:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { Office o(funds, health, hunger, level, levelPoints, ecopoints, pollutionLevel, cityName, officeUp); o.enter(); officeUp = o.getUpgradeLevel(); }
                break;
            case 8:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { Restaurant r(funds, ecopoints, hunger, health, level, levelPoints, pollutionLevel, cityName, restaurantUp); r.enter(); restaurantUp = r.getUpgradeLevel(); }
                break;
            case 9:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { School s(funds, ecopoints, health, hunger, level, levelPoints, pollutionLevel, cityName, schoolUp); s.enter(); schoolUp = s.getUpgradeLevel(); }
                break;
            case 10:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { Bank b(funds, health, hunger, level, levelPoints, ecopoints, pollutionLevel, cityName, bankUp); b.enter(); bankUp = b.getUpgradeLevel(); }
                break;
            case 11:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                { Casino c(funds, health, hunger, level, levelPoints, ecopoints, pollutionLevel, cityName, casinoUp); c.enter(); casinoUp = c.getUpgradeLevel(); }
                break;
            case 12:
                { Environment e(funds, ecopoints, pollutionLevel, level, levelPoints, health, hunger, cityName, envUp); e.enter(); envUp = e.getUpgradeLevel(); }
                break;
            case 13:
                transport_delay(vehicle, ecopoints, pollutionLevel);
                Summit(funds, ecopoints, level, pollutionLevel, currentUser, hasLoan, loanAmount, lenderName, loanStartDate).enter();
                break;
            case 14:
                { RecyclingCenter r(funds, pollutionLevel, level, levelPoints, ecopoints, health, hunger, cityName, recUp); r.enter(); recUp = r.getUpgradeLevel(); }
                break;
            case 15:
                { CommunityGarden g(funds, ecopoints, health, level, levelPoints, pollutionLevel, hunger, cityName, gardenUp); g.enter(); gardenUp = g.getUpgradeLevel(); }
                break;
            case 16:
                showNewsFeed();
                break;
            case 0:
                clearScreen();
                cout << "\n=== Game Summary ===\n";
                cout << "💰 Funds: " << funds << " (" << (netFunds >= 0 ? "+" : "") << netFunds << ")\n";
                cout << "❤️ Health: " << health << "/" << MAX_HEALTH << "\n";
                cout << "🍽️ Hunger: " << hunger << "/" << MAX_HUNGER << "\n";
                cout << "📊 Level: " << level << " (Points: " << levelPoints << "/" << LEVEL_UP_THRESHOLD << ")\n";
                cout << "🌱 Eco Points: " << ecopoints << "\n";
                cout << "🌫️ Pollution Level: " << pollutionLevel << "\n";
                cout << "===================\n\n";
                return;
            default:
                cout << "Invalid choice. Try again!\n";
                break;
        }
    }
}

// ------------ Main ------------
int main()
{
    int level = 0, ecopoints = 0, funds = 0;
    int health = MAX_HEALTH, hunger = MAX_HUNGER, levelPoints = 0;
    int houseUp = 1, hospitalUp = 1, officeUp = 1, restaurantUp = 1, schoolUp = 1, bankUp = 1, casinoUp = 1, envUp = 1, recUp = 1, gardenUp = 1;
    int vehicle = 0, pollutionlevel = 0;
    time_t lastSaveTime = time(nullptr);
    time_t lastUpdateTime = time(nullptr);
    string currentUser;
    // Loan variables
    bool hasLoan = false;
    int loanAmount = 0;
    string lenderName = "";
    string loanStartDate = "";
    string cityName = "";
    int daysWithZeroPollution = 0;  // Initialize days with zero pollution

    string name, pin, filename;
    cout << "Enter your name: ";
    getline(cin, name);
    currentUser = name;
    cout << "Enter your 4-digit PIN: ";
    cin >> pin;
    cin.ignore();

    filename = name + "_" + pin + ".txt";

    bool fileExists = load_game(filename, level, ecopoints, funds, health, hunger,
        houseUp, hospitalUp, officeUp, restaurantUp, schoolUp, bankUp, casinoUp, envUp, recUp, gardenUp,
        vehicle, pollutionlevel, lastSaveTime, levelPoints, hasLoan, loanAmount, lenderName, loanStartDate, cityName, daysWithZeroPollution);

    if (!fileExists)
    {
        displayInstructions("eco_city_instructions.txt"); // only show for new users
        level = 1;
        funds = 500;  // Give new players 500 starting funds
        cout << "Name your city: ";
        getline(cin, cityName);
    }

    offline_bonus(funds, lastSaveTime);

    // Generate initial news when game starts
    generateNews("🌱 Environmental", "Welcome to " + cityName + "!", 
                "Welcome to your eco-city! Start building a sustainable future by making environmentally conscious decisions.", true);
    generateNews("🏙️ City Development", "New City Established", 
                "A new eco-friendly city has been established. Citizens are encouraged to participate in sustainable development.", true);

    menu(funds, vehicle, level, lastUpdateTime, ecopoints, health, hunger, levelPoints, pollutionlevel, currentUser, hasLoan, loanAmount, lenderName, loanStartDate,
        houseUp, hospitalUp, officeUp, restaurantUp, schoolUp, bankUp, casinoUp, envUp, recUp, gardenUp, cityName, daysWithZeroPollution);

    save_game(filename, level, ecopoints, funds, health, hunger,
        houseUp, hospitalUp, officeUp, restaurantUp, schoolUp, bankUp, casinoUp, envUp, recUp, gardenUp,
        vehicle, pollutionlevel, time(nullptr), levelPoints, hasLoan, loanAmount, lenderName, loanStartDate, cityName, daysWithZeroPollution);

    cout << "Game saved. Goodbye!" << endl;
    return 0;
}
