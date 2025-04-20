#include "/home/omer/Desktop/421_code/hw1/main_files/Angel.h"
#include "/usr/include/GLFW/glfw3.h"
#include <vector>
#include <algorithm>  // remove_if fonksiyonu için
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

// Sidewalk constants
const int NumSidewalkPoints = 4; // Number of points per sidewalk.
const double SidewalkHeight = 30;
const int NumSidewalks = 6;

// Road constants
const int NumLinesPerRoad = 3; // Number of line rows per road.
const double DividerLineHeight = 5;
const double LineHeight = 30;
const double RoadHeight = DividerLineHeight * (NumLinesPerRoad - 1) + LineHeight * NumLinesPerRoad;
const int NumRoads = 5;

// Window constants
const double WindowHeight = SidewalkHeight * NumSidewalks + RoadHeight * NumRoads; // Pencere yuksekligi kaldirim yol ve yol cizgilerinin yukseklikleri toplanarak hesaplaniyor.
const double WindowWidth = 500;
const double AspectRatio = WindowWidth / WindowHeight;

// Scaled sizes (Scaled to (-1,1))
const double ScaledSidewalkHeight = (SidewalkHeight / WindowHeight) * 2;
const double ScaledRoadHeight = (RoadHeight / WindowHeight) * 2;
const double ScaledLineHeight = (LineHeight / WindowHeight) * 2;
const double ScaledDividerLineHeight = (DividerLineHeight / WindowHeight) * 2;

// Line constants
const int NumRoadLinePoints = 4;
const double DividerLineWidth = 15;
const double LineSpaceWidth = 10; // Space between to divider lines (horizontally)

// Vehicle constants
const double MaxVelocity = 0.04;
const double MinVelocity = 0.02;
const int NumCarPoints = 4;
const int NumVehiclePoints = 4;
const double carSize = ScaledLineHeight * 0.8; // 0.8 is for some vertical padding
int id = 0; // Vehicle ID

// Memory allocation
const unsigned long SidewalkSize = NumSidewalkPoints * NumSidewalks * sizeof(vec3); // Buffer size of sidewalks

// Coin constants
#define PI 3.14159265
const int NumCoinPoints = 102;  // Daire kenar sayisi
const double coinSize = carSize;
const double radius = carSize / 2;

// Agent constants
const int NumAgentPoints = 3;
const double agentBottomSize = carSize; // Ucgenin taban kenari uzunlugu
double agentDistanceFromEdge = 0; // Ucgenin kenara olan uzakligi
double agentHeight = carSize;
// Agent maksimum 19 kez yukari ve asagi gidebilir.
int upCount = 0; 
int downCount = 0; 

// Agent'ın bir yone gitme sayisi diger yone girme sayisindan 9 fazla olabilir. Cunku 9 * agent_width 1' esit. (Agent ortadan basliyor. En fazla 1 sola ya da saga gidebilir.)
int leftCount = 0;
int rightCount = 0;
double dividerLineOffset = 0; // How many divider line height to be added to agent coordinates
int direction = 0; // 0 -> up, 1 -> down

// Pause, single step constants
bool singleStep = false;
bool isPaused = false;
double pauseStartTime = 0;
double pauseDuration = 0;

int score = 0;

// Scale points according to aspect ratio of the window.
void myGlBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
    
    const vec3* inputData = (const vec3*)data;
    
    vec3* updatedData = new vec3[size / sizeof(vec3)];

    for (int i = 0; i < size / sizeof(vec3); i++) {
        updatedData[i].x = inputData[i].x  / AspectRatio;  
        updatedData[i].y = inputData[i].y; 
    }

    glBufferSubData(target, offset, size, updatedData);

    delete[] updatedData;
};

std::vector<vec3> allSidewalkPoints;

void initSidewalkPoints() {
    for (int i = 0; i < NumSidewalks; i++) {
        double distanceFromBottom = (ScaledRoadHeight + ScaledSidewalkHeight) * i; // Tabandan baslayarak kaldirim noktalari olusturulur.
        vec3 roadPoints[NumSidewalkPoints] = {
            vec3( 1, -1 + ScaledSidewalkHeight + distanceFromBottom, 0 ),
            vec3( -1, -1 + ScaledSidewalkHeight + distanceFromBottom, 0 ),
            vec3( 1, -1 + distanceFromBottom, 0 ),
            vec3( -1, -1 + distanceFromBottom, 0 ),
        };
        
        for (int j = 0 ; j < NumSidewalkPoints; j++) {
            allSidewalkPoints.push_back(roadPoints[j]);
        }
    }
}

void paintSidewalks() {
    for ( int i = 0; i < NumSidewalks; i++) {
        glBufferSubData(GL_ARRAY_BUFFER, i * ( NumSidewalkPoints * sizeof(vec3) ), NumSidewalkPoints * sizeof(vec3), &allSidewalkPoints[ i * NumSidewalkPoints ]);
        glDrawArrays(GL_TRIANGLE_STRIP, i * NumSidewalkPoints, NumSidewalkPoints);
    }
}

std::vector<vec3> allRoadLinePoints;

// Her yoldaki divider line'larin noktalarini olusturur.
void initRoadLinePoints() {
    for (int i = 0; i < NumRoads; i++) {
        for (int j = 0; j < NumLinesPerRoad -1 ; j++) { // Every road has three rows of lines
            double endY =  1 - (ScaledSidewalkHeight * (i + 1) + (ScaledRoadHeight * i) + (ScaledDividerLineHeight * j) + (ScaledLineHeight) * (j + 1)); // Dikdortgenin yukaridaki y koordinati.
            double lineHeightScaled = ( DividerLineHeight / WindowHeight ) * 2;
            double dividerLineEndPixel = WindowWidth / 2 - LineSpaceWidth / 2;

            while ( dividerLineEndPixel - DividerLineWidth  > - (WindowWidth / 2) ) { // Pencere genisliginde yer kalana kadar devam et.
                double endX = ( dividerLineEndPixel / WindowWidth ) * 2;
                double lineWidthScaled = ( DividerLineWidth / WindowWidth ) * 2;
                
                vec3 linePoints [NumRoadLinePoints] = {
                    vec3( endX, endY, 0 ),
                    vec3( endX - lineWidthScaled, endY, 0 ),
                    vec3( endX, endY - lineHeightScaled, 0 ),
                    vec3( endX - lineWidthScaled, endY - lineHeightScaled, 0 )
                };
                dividerLineEndPixel -= DividerLineWidth + LineSpaceWidth;

                for (int k = 0 ; k < NumRoadLinePoints; k++) {
                    allRoadLinePoints.push_back(linePoints[k]);
                }
            }
        }
    }
}

void paintRoadLines() {
    for ( int i = 0; i < allRoadLinePoints.size() / NumRoadLinePoints; i++) {
        glBufferSubData(GL_ARRAY_BUFFER, i * ( NumRoadLinePoints * sizeof(vec3) ), NumRoadLinePoints * sizeof(vec3), &allRoadLinePoints[ i * NumRoadLinePoints ]);
        glDrawArrays(GL_TRIANGLE_STRIP, i * NumRoadLinePoints, NumRoadLinePoints);
    }  
}

class Line {
    public:
    int vehicleDirection; // LR -> 0, RL -> 1
    double lineVelocity; // Every vehicle on a lane must have the same velocity which is randomly selected (MinVelocity, MaxVelocity). 
    int numVehicles; // Eger numVehicles 0 ise o yola farklı yonde yeni bir arac eklenebilir.
    bool hasCoin; // her lane'de max bir coin olacak sekilde ayarlamak istedim.

    public:
    Line(int vehicleDirection, int numVehicles) {
        this->vehicleDirection = vehicleDirection;
        this->lineVelocity = MinVelocity + (std::rand() / (RAND_MAX / (MaxVelocity - MinVelocity)));
        this->numVehicles = 0;
        this->hasCoin = false;
    }
};

std::vector<Line> lines;

void initLines() {
    for (int i = 0; i < NumRoads * NumLinesPerRoad; i++) {
        lines.emplace_back(0, 0);  // Her nesneyi bağımsız olarak oluştur
    }
}

class Vehicle {
public:
    int id; // Araclari vehicle vectorunden silmek icin
    int roadNo;
    int lineNo;
    int direction; // 0 => LR, 1 => RL
    double velocity;
    double width;
    double height;
    vec3 coordinates[NumVehiclePoints];
    double lastUpdateTime; // Belirli araliklarla arac ilerler

    Vehicle(int id, int roadNo, int lineNo, int direction, double velocity, double width, double height)
        : id(id), roadNo(roadNo), lineNo(lineNo), direction(direction), velocity(velocity), width(width), height(height), lastUpdateTime(glfwGetTime()) {
        
        // Arac ilk olusturuldugunda x koordinatlari aracin yonune gore -1 ya da 1'dir. (Arac hareket etmeye basladiginda aracin bir kismi gorunecegi icin.)
        double leftX = direction == 0 ? -1 : 1; 
        double rightX = direction == 0 ? -1 : 1;

        // Aracin yol ve line sırasina gore y koordinati hesaplaniyor.
        double topY = 1 - (ScaledSidewalkHeight * (roadNo + 1) + (ScaledRoadHeight * roadNo) + ((ScaledLineHeight - height) / 2) + (lineNo * (ScaledLineHeight + ScaledDividerLineHeight)));

        setCoordinates(
            vec3(rightX, topY, 0),
            vec3(leftX, topY, 0),
            vec3(rightX, topY - height, 0),
            vec3(leftX, topY - height, 0)
        );
    }

    virtual void setCoordinates(vec3 topRight, vec3 topLeft, vec3 bottomRight, vec3 bottomLeft) {
        coordinates[0] = topRight;
        coordinates[1] = topLeft;
        coordinates[2] = bottomRight;
        coordinates[3] = bottomLeft;
    }

    virtual void moveLeft(double distance, std::vector<Vehicle*>& vehicles) {
        bool leaving = coordinates[1].x - distance < -1; // Pencereden ayriliyor mu?
        double newLeftX = leaving ? -1 : coordinates[1].x - distance; // Pencereden ayriliyorsa sol kenarinin x koordinatlari -1 yapilabilir.
        double newRightX;

        if (leaving) {
            newRightX = coordinates[0].x - distance; // distance yani hiz*zaman kadar sola git
        } else if (newLeftX + width >= 1) { // Tamami cikamadi
            newRightX = 1;
        } else { // Hic cikmamisken bir anda hepsi cikti
            newRightX = newLeftX + width;
        }

        if (newRightX <= -1) { // Aracin tamami ekrandan cikmis. Araci sil.
            vehicles.erase(std::remove_if(vehicles.begin(), vehicles.end(),
                                          [this](Vehicle* v) { return v->id == this->id; }),
                           vehicles.end());
            lines[roadNo * NumLinesPerRoad + lineNo].numVehicles--;
        }

        setCoordinates(
            vec3(newRightX, coordinates[0].y, 0),
            vec3(newLeftX, coordinates[1].y, 0),
            vec3(newRightX, coordinates[2].y, 0),
            vec3(newLeftX, coordinates[3].y, 0)
        );
    }

    virtual void moveRight(double distance, std::vector<Vehicle*>& vehicles) {
        bool leaving = coordinates[0].x + distance > 1;
        double newRightX = leaving ? 1 : coordinates[0].x + distance; // Pencereden ayriliyorsa sag kenarinin x koordinati 1 olmali.
        double newLeftX;

        if (leaving) {
            newLeftX = coordinates[1].x + distance;
        } else if (newRightX - width <= -1) { // Tamami cikamadi
            newLeftX = -1;
        } else { // Hic cikmamisken bir anda hepsi cikti
            newLeftX = newRightX - width;
        }
 
        if (newLeftX >= 1) { // Araci sil.
            vehicles.erase(std::remove_if(vehicles.begin(), vehicles.end(),
                                          [this](Vehicle* v) { return v->id == this->id; }),
                           vehicles.end());
            lines[roadNo * NumLinesPerRoad + lineNo].numVehicles--;
        }

        setCoordinates(
            vec3(newRightX, coordinates[0].y, 0),
            vec3(newLeftX, coordinates[1].y, 0),
            vec3(newRightX, coordinates[2].y, 0),
            vec3(newLeftX, coordinates[3].y, 0)
        );
    }

    virtual ~Vehicle() = default;
};

class Car : public Vehicle {
public:
    Car(int id, int roadNo, int lineNo, int direction, double velocity)
        : Vehicle(id, roadNo, lineNo, direction, velocity, carSize, carSize) {}
};

class Truck : public Vehicle {
public:
    Truck(int id, int roadNo, int lineNo, int direction, double velocity)
        : Vehicle(id, roadNo, lineNo, direction, velocity, carSize * 2, carSize) {} // Width = 2 * Height
};

std::vector<Vehicle*> vehicles;

void drive() {
    double currentTime = glfwGetTime();
    double updateInterval = 0.1; // Aracin konumunu guncelleme araligi

    for (int i = 0; i < vehicles.size(); i++) {
        if (currentTime - pauseDuration - vehicles[i]->lastUpdateTime >= updateInterval) { // Aracin konum guncelleme zamani geldi mi
            double velocity = vehicles[i]->velocity;

            if (vehicles[i]->direction == 0) {
                vehicles[i]->moveRight(velocity, vehicles);
            } else {
                vehicles[i]->moveLeft(velocity, vehicles);
            }

            int roadNo = vehicles[i]->roadNo;
            int lineNo = vehicles[i]->lineNo;
            vehicles[i]->lastUpdateTime = currentTime - pauseDuration; // Reset update time
        }
    }
}

void paintCars(){
       for (int i = 0; i < vehicles.size(); i++) {
        myGlBufferSubData(GL_ARRAY_BUFFER, i * NumCarPoints * sizeof(vec3), NumCarPoints * sizeof(vec3), &vehicles[i]->coordinates);
        glDrawArrays(GL_TRIANGLE_STRIP, i * NumCarPoints, NumCarPoints);
    }
}

void initRandomCars() {
    int attempts = 5;  // Max attempts, bir yola zit yonde arac eklenmek isteniyor olabilir. Lane'ler ve yonler rastgeledir.
    while (attempts--) {
        int roadNo = std::rand() % 5; // Max 5 yol.
        int lineNo = std::rand() % 2; // Max 3 lane.
        int direction = std::rand() % 2; // 0 -> RL, 1 -> LR
        int carType = std::rand() % 2; // 0 -> car, 1-> truck

        int index = roadNo * NumLinesPerRoad + lineNo;
        if (index < 0 || index >= lines.size()) continue; // Out-of-bounds check

        Line& line = lines[index]; // Aracin eklenecegi line objesi

        double velocity = line.lineVelocity;

        if ((line.vehicleDirection == direction || line.numVehicles == 0)) { // O line'da hareket eden aracla ayni yondeyse veya o line bossa yeni arac bu yola eklenebilir.
            line.vehicleDirection = direction;
            line.numVehicles++;
            if (carType == 0) {
                vehicles.push_back(new Car(id++, roadNo, lineNo, direction, velocity));
            } else {
                vehicles.push_back(new Truck(id++, roadNo, lineNo, direction, velocity));
            }
            return;
        }
    }
}

class Coin {
    public: 
        double spawnTime;
        int roadNo;
        int lineNo;
        vec3 coordinates[NumCoinPoints];

    public:
    Coin(double spawnTime, int roadNo, int lineNo, vec3* initCoordinates){
        this->spawnTime = spawnTime;
        this->roadNo = roadNo;
        this->lineNo = lineNo;

        for ( int i = 0; i<NumCoinPoints ; i++) {
            coordinates[i] = initCoordinates[i];
        };
    }

    void checkDuration(std::vector<Coin*>& coins) {
        // Coin 3 saniyedir duruyorsa coini sil.
        coins.erase(std::remove_if(coins.begin(), coins.end(),
                        [this](Coin* c) { return glfwGetTime() - pauseDuration - c->spawnTime > 3; }),
            coins.end());
        lines[roadNo * NumLinesPerRoad + lineNo].hasCoin = false;
    }
};

std::vector<Coin*> coins;

void initCoins() {
    float dTheta = 2 * PI / (NumCoinPoints -2); // NumCoinPoints'e merkez ve tam bir daire olmasi icin +1 nokta daha eklendi. Bu yuzden 102 - 2 = 100 kenarli.
    vec3 points[NumCoinPoints];
    vec3 coordinates[NumCoinPoints];
    int attempts = 3; // Bir satira birden fazla coin eklenmeye calisiliyor olabilir.

    while(attempts-- > 0) {
        int roadNo = rand() % NumRoads;
        int lineNo = rand() % NumLinesPerRoad;

        if (lines[roadNo * NumLinesPerRoad + lineNo].hasCoin) // Her satir tek coin'e sahip olabilir.
            continue;
        else    
            lines[roadNo * NumLinesPerRoad + lineNo].hasCoin = true;

        int distFromCenter = rand() % 10; // Sola ya da saga en fazla 9 * Coin_Size kadar uzak olabilir. Yoksa pencereden cikar.
        int leftOrRight = rand() % 2 == 0 ? -1 : 1; // -1 left, 1 right
        double x = distFromCenter * carSize * leftOrRight;
    
        double y = 1 - (roadNo * ScaledRoadHeight + (roadNo+1) * ScaledSidewalkHeight + lineNo * ScaledLineHeight  + (lineNo)*ScaledDividerLineHeight + ( ScaledLineHeight / 2 ));

        points[0] = vec3(x, y, 1); // Center point

        for (int i = 1; i < NumCoinPoints-1; i++) {
            float theta = i * dTheta;
            float x2 = radius * cos(theta);
            float y2 = radius * sin(theta);
            points[i] = vec3(x + x2, y + y2, 1);
        }

        points[NumCoinPoints-1] = points[1];

        for (int i = 0; i < NumCoinPoints; i++) {
            coordinates[i] = points[i];
        }

        double spawnTime = glfwGetTime();  
        coins.push_back(new Coin(spawnTime, roadNo, lineNo, coordinates));
        return;
    }
}

void checkCoins() {
    for( int i = 0; i<coins.size(); i++) {
      coins[i]->checkDuration(coins); // Hayatta kalma sureleri bitti mi kontrol et.
    }
}

void paintCoins() {
    for( int i = 0; i<coins.size(); i++) {
        myGlBufferSubData(GL_ARRAY_BUFFER, (NumCoinPoints) * sizeof(vec3) * i, (NumCoinPoints) * sizeof(vec3), &coins[i]->coordinates);
        glDrawArrays(GL_TRIANGLE_FAN, NumCoinPoints * i, NumCoinPoints);
    }
}

void goUp() {
    if (upCount == 19) { // 19 * Agent_Height = 2. 
        direction = 1; // Asagi dondur.
        upCount = 0;
        return;
    }
    agentDistanceFromEdge += (ScaledLineHeight * AspectRatio); // Distance from bottom.
    upCount++;
    if (upCount % 4 == 3 || upCount % 4 == 2) // Bir yolda, orta ya da en ust lane e gecmek icin yol yuksekligi + divider line yulsekligi lazim. Divider line yuksekligini burada hesapliyorum.
        dividerLineOffset+= ScaledDividerLineHeight;
   
}

void goDown() {
    if (downCount == 19) {
        direction = 0; // Yukari dondur.
        downCount = 0;
        return;
    }
    agentDistanceFromEdge -= (ScaledLineHeight * AspectRatio); // Distance from bottom.
    downCount++;
    if (downCount % 4 == 3 || downCount % 4 == 2)
        dividerLineOffset -= ScaledDividerLineHeight;
   
}

void goLeft() {
    if ((leftCount - rightCount) < 10) { // Coin gibi en fazla 9 * Agent_Size kadar saga ya da sola gidilebilir.
        leftCount++;
    }
}

void goRight() {
    if ((rightCount - leftCount) < 10) {
        rightCount++;
    }
}

vec3 agentPoints[NumAgentPoints];

void initAgent() {
    // Agent'in yonune ve upCount ve downCount'a gore y koordinati belirlenir. En sondaki toplanan eleman vertical paddingtir. 
    double y  = -1 + (ScaledLineHeight * (direction == 0 ? upCount : 20 - downCount) + dividerLineOffset) + (direction == 0 ? 0.005 : -0.005);
    double x = agentBottomSize * (rightCount - leftCount);

    if (direction == 0) { 
        // Agent'in yuksekligi tabanin 1.5 kati.
        agentPoints[0] = vec3(x - agentBottomSize / 2, y, 1);
        agentPoints[1] = vec3(x + agentBottomSize / 2, y, 1);
        agentPoints[2] = vec3(x, agentBottomSize * 3 / 2 * 0.8 + y, 1);
    } else { // Rotate agent
        agentPoints[0] = vec3(x - agentBottomSize / 2, agentBottomSize * 3 / 2 * 0.8 + y, 1);
        agentPoints[1] = vec3(x + agentBottomSize / 2, agentBottomSize * 3 / 2 * 0.8 + y, 1);
        agentPoints[2] = vec3(x, y, 1);
    }

    myGlBufferSubData(GL_ARRAY_BUFFER, 0 , (NumAgentPoints) * sizeof(vec3), agentPoints);
    glDrawArrays(GL_TRIANGLES, 0 , NumAgentPoints);

}

GLenum glCheckError() {
    const char *msgs[] = {
        "No error", 
        "INVALID_ENUM", 
        "INVALID_VALUE", 
        "INVALID_OPERATION", 
        "STACK_OVERFLOW", 
        "STACK_UNDERFLOW", 
        "INVALID_FRAMEBUFFER_OPERATION"
    };

    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        if (errorCode >= GL_INVALID_ENUM && errorCode <= GL_INVALID_FRAMEBUFFER_OPERATION) {
            // Hata kodu geçerli aralıktaysa, doğru mesajı yazdır
            printf("%s | Error Code: %d\n", msgs[errorCode - GL_INVALID_ENUM + 1], errorCode);
        } else {
            // Tanımlanamayan hata kodu
            printf("Unknown error code: %d\n", errorCode);
        }
    }
    return errorCode;
}

void init( GLuint vbo ) {

    // Init traffic points.
    initSidewalkPoints();
    initRoadLinePoints();
    initLines();

    // Buffer sizes
    unsigned long roadLineSize = allRoadLinePoints.size() * sizeof(vec3);
    unsigned long carSize = NumCarPoints * sizeof(vec3) * vehicles.size();
    unsigned long coinPointSize = coins.size() * NumCoinPoints * sizeof(vec3);
    unsigned long trafficSize = roadLineSize + carSize + SidewalkSize;
    unsigned long agentSize = sizeof(vec3) * NumAgentPoints;

    // VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, coinPointSize + trafficSize + agentSize, NULL, GL_STATIC_DRAW);

    // Load Shaders
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // Initialize vertex attributes
    GLuint pos = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnable(GL_DEPTH_TEST);
    glCheckError();
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void pauseGame() {
    isPaused = true;
    pauseStartTime = glfwGetTime();
}

void continueGame() {
    isPaused = false;
    pauseDuration += glfwGetTime() - pauseStartTime; // Simdiye kadar olan pauseDuration'a yenisini ekle.
}

void reshape( GLFWwindow *w, int width, int height ) {
    glViewport( 0, 0, width, height ); // Sol ust 
}

void printInfo() {
    int totalVehicles = 0;
    std::cout << "Vehicle konumlari: " << std::endl;
    for (int i = 0 ; i < vehicles.size(); i++) {
        double corner1 = vehicles[i]->coordinates[0].x;
        double corner2 = vehicles[i]->coordinates[1].x;
        if (vehicles[i]->coordinates[0].x != vehicles[i]->coordinates[1].x) {
            totalVehicles++;
            for (int j = 0; j < 4 ; j++) {
                std::cout << vehicles[i]->coordinates[j].x << ", " <<  vehicles[i]->coordinates[j].y << std::endl;
            }
                std::cout << "------" << std::endl;
        }
    }

   std::cout << "Yoldaki mevcut arac sayisi: " << totalVehicles << std::endl;
    std::cout << "Agent konumu: " << std::endl;
    for (int i = 0 ; i< NumAgentPoints ; i++) {
       std::cout << "( " << agentPoints[i].x << ", " << agentPoints[i].y << " ), ";
    };
    std::cout << std::endl;
}
void keyboard( GLFWwindow *w, int key, int scancode, int action, int mods ) {  
    switch (key) {
    case GLFW_KEY_Q:
      glfwSetWindowShouldClose(w, GL_TRUE);
      break;
      case GLFW_KEY_UP:
      if (action == GLFW_PRESS && direction == 0 && !isPaused)
        goUp();
      break;  
         case GLFW_KEY_DOWN:
      if (action == GLFW_PRESS && direction == 1 && !isPaused)
        goDown();
      break;  
       case GLFW_KEY_LEFT:
      if (action == GLFW_PRESS && !isPaused)
        goLeft();
      break;  
       case GLFW_KEY_RIGHT:
      if (action == GLFW_PRESS && !isPaused)
        goRight();
      break;  
      case GLFW_KEY_S:
        if (action == GLFW_PRESS) {
            singleStep = true;
        };
        break;
      case GLFW_KEY_P:
      if (action == GLFW_PRESS) {
        if (isPaused) {
            continueGame();
            printInfo();
        } else {
            pauseGame();
        }
      }
     }
}


// Zaman değişkenleri
double lastVehicleCreationTime = glfwGetTime();
double lastCoinCreationTime = glfwGetTime();
double randomVehicleInterval;
double randomCoinInterval;

// Zamanı güncelleyen fonksiyon
void updateCarSpawn() {
    double currentTime = glfwGetTime() - pauseDuration;


    if (currentTime - lastVehicleCreationTime > randomVehicleInterval && !isPaused) {
        randomVehicleInterval = 1.0 + (rand() % 4); // 4 saniyede bir yeni arac olustur.
        lastVehicleCreationTime = currentTime;
        for (int i = 0; i < 4; i++) { initRandomCars(); } // 5 tane arac olustur.
    }

    if (currentTime - lastCoinCreationTime > randomCoinInterval && !isPaused) {
        randomCoinInterval = 1.0 + (rand() % 5); // 5 saniyede bir coin olustur.
        lastCoinCreationTime = currentTime;
        initCoins();
    }
}

// Oyun mantığını ve çizimleri güncelleyen fonksiyon
void updateGameState(GLFWwindow* window, GLuint vbo) {
    updateCarSpawn();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    paintSidewalks();
    paintRoadLines();
    paintCars();
    initAgent();
    paintCoins();

    if (!isPaused) {
        drive();
        checkCoins();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}



// Oyun başlatma ve ana döngü
int main(int argc, char **argv) {
    GLFWwindow* window;

    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WindowWidth, WindowHeight, "hw1", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    GLuint vbo;
    init(vbo);  // OpenGL başlangıç işlemleri

    glfwSetKeyCallback(window, keyboard);  // Klavye olayları
    glfwSetWindowSizeCallback(window, reshape);

    // Ana döngü
    while (!glfwWindowShouldClose(window)) {

        if (singleStep) {
            if (isPaused)
            continueGame();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printInfo();
            updateGameState(window, vbo);
            singleStep = false;
            pauseGame();
        } else {
            updateGameState(window, vbo);
        }
    }

    glfwTerminate();
    return 0;
}