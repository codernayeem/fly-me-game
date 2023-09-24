#include <iostream>
#include <list>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <fstream>

using namespace std;

const int height = 15;
const int width = 80;
long long highest_score[] = {0, 0, 0};
const string DIFFICULTY[] = {"Easy", "Normal", "Hard"};

const string fileName = "highest_score.txt";

const string RESET = "\033[0m";
const string BLACK = "\033[30m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";

//💥💥☠️💀👽🍎🍏🎃🏆✈️🚁🛩️🔥⚡🔑🟡🏁🪙💲💵✴️◀️💣

const int BombCount = 3;
string BOMBS[BombCount] = {"💥", "💀", "🚁"};

const int CoinsCount = 3;
string COINS[CoinsCount] = {"🟡", "🍏", "💲"};

const int ArrowCount = 2;
string ARROWS[ArrowCount] = {"◀", "⚡"};

const int WINGS_POINT_CUT_RANGE[] = {200, 400};
const int WINGS_POINT_ADD_RANGE[] = {500, 1000};
const int DIRECT_POINT_ADD = 1000;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool getYN(){
    char c;
    while(true){
        c = getch();
        if(c == 'y' || c == 'Y') return true;
        if(c == 'n' || c == 'N') return false;
    }
}

int getMainInput(){
    char c;
    while(true){
        c = getch();
        if(c == '1') return 1;
        if(c == '2') return 2;
        if(c == '3') return 3;
        if(c == 'q') return 0;
    }
}

template<class T>
class LastPoints
{
    int counter;

public:
    LastPoints() : counter(0){ }

    void update(){
        // show last point update for 4 sec
        if(counter == 40){
            clear();
            counter = 0;
        }else
            counter++;
    }

    void set(T data, string color){
        clear();
        gotoxy(10, 1);
        cout << color << data << RESET;
        counter = 0;
    }

    void clear(){
        gotoxy(10, 1);
        cout << "_____";
    }
};

class Game;

class Player {
private:
    int x, y;
    long long score;
    bool dead;
public:

    static vector<string> plane;

    Player() {}
    Player(int startX, int startY) : x(startX), y(startY), score(0), dead(false) {}

    void die(){
        dead = true;
    }

    void draw() {
        gotoxy(x, y);
        cout << GREEN << plane[0] << RESET;
        gotoxy(x+2, y+1);
        cout << CYAN << plane[1] << RESET;
        gotoxy(x+2, y-1);
        cout << CYAN << plane[2] << RESET;
    }

    //   \
    // ===>
    //   /

    void erase() {
        gotoxy(x, y);
        cout << "    ";
        gotoxy(x+2, y+1);
        cout << " ";
        gotoxy(x+2, y-1);
        cout << " ";
    }

    void moveUp() {
        if (y > 3) {
            erase();
            y--;
        }
    }

    void moveDown() {
        if (y < height-1) {
            erase();
            y++;
        }
    }

    void destroyAnimation(){
        string anim[10][3] = {
            {"--->", "/", "\\"},
            {"--->", "/", "\\"},
            {"- -=", "-", "-"},
            {"- -=", "-", "-"},
            {"- -=", "*", "*"},
            {"- -=", "*", "*"},
            {"- -=", "*", "*"},
            {"- - ", " ", " "},
            {"    ", " ", " "},
        };
        for(int i = 0; i < 10; i++){
            gotoxy(x, y);
            cout << GREEN << anim[i][0] << RESET;
            gotoxy(x+2, y+1);
            cout << CYAN << anim[i][1] << RESET;
            gotoxy(x+2, y-1);
            cout << CYAN << anim[i][2] << RESET;
            gotoxy(0, 0);
            Sleep(120);
        }
    }

    // operator overloading via member function
    void operator+(int score){
        if(!dead){
            this->score += score; 
        }
    }

    // operator overloading via member function
    void operator-(int score){
        if(!dead){
            score -= score;
            if(score < 0)
                score = 0;
        }
    }

    template<class T>
    T isColided(int obj_x, int obj_y){
        if(y == obj_y && x + 4 >= obj_x && x <= obj_x){
            return 1; // direct hit
        }else if((y+1 == obj_y && x + 3 >= obj_x && x+2 <= obj_x) || (y-1 == obj_y && x + 3 >= obj_x && x+2 <= obj_x)){
            return 2; // on wings
        }
        return 0;
    }

    friend class Game;
};

LastPoints<string> lastPoints;
vector<string> Player::plane = vector<string>({"===>", "/", "\\"});

class Object {
public:
    int x, y;
    string symbol;
    Object() {x=0; y=0;}
    Object(int x, int y, string symbol) : x(x), y(y), symbol(symbol){}

    // draw
    friend ostream& operator<<(ostream& out, Object *obj){
        gotoxy(obj->x, obj->y);
        out << obj->symbol;
        return out;
    }

    void erase() {
        gotoxy(x, y);
        cout << " ";
    }

    // virtual function
    virtual bool move(){
        if (!x) return false;
        x--;
        return true;
    }

    // pure virtual function
    virtual bool detectColide(Player *p) = 0;
};

class Arrow: virtual public Object{
public:
    Arrow () {}
    Arrow(int startX, int startY) : Object(startX, startY, ARROWS[rand() % ArrowCount]) {}

    bool detectColide(Player *p){
        short c = p->isColided<short>(x, y);
        if(c == 1){
            p->die();
            return true;
        }else if(c == 2){
            int r = (200 + rand() % 301);
            *p - r;
            lastPoints.set("-" + to_string(r), RED);
            return true;
        }
        return false;
    }
};

class Bomb: virtual public Object{
public:
    Bomb () {}
    Bomb(int startX, int startY) : Object(startX, startY, BOMBS[rand() % BombCount]) {}

    // copy constructor
    Bomb(const Bomb &b){
        x = b.x;
        y = b.y-1; // create new bomb at bottom
        symbol = b.symbol; // same symbol
    }

    bool detectColide(Player *p){
        if(p->isColided<bool>(x, y)){
            p->die();
            return true;
        }
        return false;
    }
};

class Coin: virtual public Object{
public:
    Coin () {}
    Coin(int startX, int startY) : Object(startX, startY, COINS[rand() % CoinsCount]) {}

    bool detectColide(Player *p){
        short c = p->isColided<short>(x, y);
        if(c == 1){
            *p - 1000;
            lastPoints.set("+1000", GREEN);
            return true;
        }else if(c == 2){
            int r = (500 + rand() % 501);
            *p + r; // coin means 500-1000 points
            lastPoints.set("+" + to_string(r), GREEN);
            return true;
        }
        return false;
    }
};

// Mystry Item That can be Arrow, Bomb, Coin or Nothing
class Mystry: public Coin, public Bomb, public Arrow{
public:
    Mystry() { }
    // default aurguments
    Mystry(int startX, int startY, string symbol = "🎃") : Object(startX, startY, symbol) {}

    bool move() {
        // it disapper early
        if (x-3 == 0) return false;
        x--;
        return true;
    }

    bool detectColide(Player *p){
        switch(rand() % 4){
            case 0:
                return Bomb::detectColide(p);
            case 1:
                return Arrow::detectColide(p);
            case 2:
                return Coin::detectColide(p);
            default:
                return false;
        }
    }
};


class Game {
    Player *player;
    list<Object*> objects;
    int diffLevel;
public:

    Game(int diffLevel) {
        this->diffLevel = diffLevel;
        player = new Player(2, (height+2)/2); // place at vertically center

        system("cls");
        gotoxy(0, 1);
        cout << "________________________________________________________________________________";
        gotoxy(0, height+1);
        cout << "________________________________________________________________________________";
        
        gotoxy(25, 0);
        cout << "Highest SCORE : " << highest_score[diffLevel];
        gotoxy(55, 0);
        cout << "Difficulty : " << DIFFICULTY[diffLevel];
        gotoxy(5, 0);
        cout << "SCORE : ";
    }

    ~Game(){
        for (auto it = objects.begin(); it != objects.end(); it++)
            delete *it;
        delete player;
    }

    void saveHighScore(){
        ofstream fout(fileName);
        fout << highest_score[0] << " " << highest_score[1] << " " << highest_score[2];
        fout.close();
    }

    void printScore(){
        gotoxy(13, 0);
        cout << "          ";
        gotoxy(13, 0);
        cout << player->score;
    }

    // update frame, return false for gameover
    bool operator++() {
        for (auto it = objects.begin(); it != objects.end();) {
            if((*it)->detectColide(player)){
                (*it)->erase();
                delete *it;
                it = objects.erase(it);
            } else {
                ++it;
            }
            if (player->dead)
                return false; // no next frame
        }

        // erase & move forward
        for (auto it = objects.begin(); it != objects.end();) {
            (*it)->erase();
            if (!(*it)->move()) {
                delete *it;
                it = objects.erase(it);
            } else {
                ++it;
            }
        }

        int percentage[3][3] = {
            {4, 3, 7},
            {5, 8, 4},
            {4, 15, 1},
        };
        int totalPercentage = 100;
        int maxObjectInBoard[3] = {20, 30, 40};

        // add new objects like arrow, bomb, coin
        int h = rand() % (height-2)+2;
        if(objects.size() < maxObjectInBoard[diffLevel]){
            int r = rand() % (totalPercentage);
            // add arrow
            if(r < percentage[diffLevel][0])
                objects.push_back(new Arrow(width-1, h));
            // add bomb
            else if(r < percentage[diffLevel][0] + percentage[diffLevel][1]){
                Bomb *b = new Bomb(width-1, h);
                objects.push_back(b);

                if(diffLevel == 2 && h > 4 && rand() % 5 == 0){
                    // use copy constructor for new bomb of same symbol
                    objects.push_back(new Bomb(*b));
                }
            }
            // add coins
            else if(r < percentage[diffLevel][0] + percentage[diffLevel][1] +  percentage[diffLevel][2])
                objects.push_back(new Coin(width-1, h));
            // mystry item 1% chance
            else if(diffLevel == 2 && r < percentage[diffLevel][0] + percentage[diffLevel][1] + percentage[diffLevel][2] + 1){
                objects.push_back(new Mystry(width-1, h));
            }
        }

        for (auto it: objects){
            cout << it; // draw objects
        }

        // draw player
        player->erase();
        player->draw();
        *player + 1;
        printScore();
        lastPoints.update();
        return true; // next frame avaiable
    }

    void gameOver() {
        player->destroyAnimation();
        Sleep(1000);
        system("cls");
        fflush(stdin);

        cout << "\n\n\t\tGame Over! Score : " << GREEN << player->score << RESET << "  [" << DIFFICULTY[diffLevel] << " Mode]" << endl;
        if(highest_score[diffLevel] < player->score){
            cout << "\t\tNew Highest Score! You were ahead by " <<  player->score - highest_score[diffLevel] << " points!" << endl;
            highest_score[diffLevel] = player->score;
            saveHighScore();
        }else{
            int z = player->score * 100 / highest_score[diffLevel];
            cout << "\t\tHighest Score : " << highest_score[diffLevel] << endl;
            cout << "\t\tYou were " << z << " % close to the highest score!" << endl;
        }
    }

    // static Function
    static void loadHighestScore(){
        ifstream fin(fileName);
        if(!fin){
            fin.close();
            ofstream fout(fileName);
            fout << "0 0 0";
            fout.close();
        }else{
            fin >> highest_score[0] >>  highest_score[1] >>  highest_score[2];
            fin.close();
        }
    }

    friend void startAGame(int);
};


// friend function of Game Class
void startAGame(int diffLevel){
    Game game(diffLevel);

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 'w') {
                game.player->moveUp();
            } else if (key == 's') {
                game.player->moveDown();
            }
        }
        
        // update Frame
        if(++game == false){
            game.gameOver();
            break;
        }

        gotoxy(0, 0);
        Sleep(100);
    }
}

void printFirstScreen(){
    system("cls");
    cout << "\n\t\t\t\t"<<"🛩️"<<" FLY ME "<<"🛩️"<<"\n";
    cout << "\t\t\t"<<"🛩️"<<" A Project by Roll: " << GREEN << "2107050 " << RESET <<"🛩️"<<"\n\n";
    cout << CYAN << "\t\t\t\t     "<<Player::plane[2]<<"\n" << RESET;
    cout << GREEN <<"\t\t\t\t   "<< Player::plane[0] <<"\n" << RESET;
    cout << CYAN << "\t\t\t\t     "<< Player::plane[1] <<"" << RESET;
    cout << "\n\tRules: ";
    cout << "\n\t\t1. "<<"💥"<<"  "<<"💀"<<"  "<<"🚁"<<"  => Touch them: You " << RED << "DIE" << RESET <<"☠️";
    cout << "\n\t\t2. "<<"◀"<<"  "<<"⚡"<<"       => On wings  : Decrease score by (" << RED << "200-300" << RESET <<")";
    cout << "\n\t\t3. "<<"◀"<<"  "<<"⚡"<<"       => Direct hit: You " << RED << "DIE" << RESET <<"☠️";
    cout << "\n\t\t4. "<<"🟡"<<"  "<<"🍏"<<"  "<<"💲"<<"  => On wings  : Increase score by (" << GREEN << "500-1000" << RESET <<")";
    cout << "\n\t\t5. "<<"🟡"<<"  "<<"🍏"<<"  "<<"💲"<<"  => Direct hit: Increase score by " << GREEN << "1000" << RESET;
    cout << "\n\t\t6. "<<"🎃"<<"        "<<"  => "<< MAGENTA << "Mystry" << RESET << " Item: Can be " << MAGENTA << "AYTHING" << RESET << " [Hard Mode only]";

    cout << "\n\n\tEnter (1 / 2 / 3) to start the Game in (Easy / Normal / Hard) Mode\n";
    cout << "\n\t\t            Enter 'q' to Quit the Game\n";
    cout << "\n\t\t\t      Highest Scores :";
    cout << "\n\t\t\t          Easy       : " << highest_score[0];
    cout << "\n\t\t\t          Normal     : " << highest_score[1];
    cout << "\n\t\t\t          Hard       : " << highest_score[2];
    cout << endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    SetConsoleOutputCP(CP_UTF8);
    Game::loadHighestScore();
    printFirstScreen();

    int diffLevel = 0;
    while(diffLevel = getMainInput()){
        do{
            startAGame(diffLevel-1);
            cout << "\n\n\t\tPlay Again (y/n)?" << endl;
            fflush(stdin);
        }while(getYN());
        
        printFirstScreen();
    }
    cout << endl;
    return 0;
}
