// GPIO35 - Up-Down - Move
// GPIO32 - BTN_A - RGB Traka
// GPIO33 - BTN_B - OK

#include "Adafruit_ILI9341.h"
#include "Adafruit_GFX.h"

#include <Adafruit_NeoPixel.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN    32   // GPIO32 za LED-ice
#define LED_COUNT  5   // Primijeniti broj na broj LEDica na vašoj traci
#define SPEAKER_PIN 25  // GPIO25 za zvucnik

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Define the player color
 #define Player_color_red
// #define Player_color_green
// #define Player_color_blue

// Configure WiFi based on player color
#ifdef Player_color_red
  const char* ssid = "VIDIX1";
  const char* password = "vidix1234";  // mora imati 8+ znakova
  String Player_color = "red";
#elif defined(Player_color_green)
  const char* ssid = "VIDIX2";
  const char* password = "vidix1234";  // mora imati 8+ znakova
  String Player_color = "green";
#elif defined(Player_color_blue)
  const char* ssid = "VIDIX3";
  const char* password = "vidix1234";  // mora imati 8+ znakova
  String Player_color = "blue";
#endif

  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  
AsyncWebServer server(80);

// Define pin connections and other constants
#define BTN_READ 13  // Button to read scores
#define BUTTON_PIN 39  // Definicija pina na kojem je spojen gumb START

#define TFT_BLACK 0x0000
#define TFT_BLUE 0x001F
#define TFT_RED 0xF800
#define TFT_GREEN 0x07E0
#define TFT_CYAN 0x07FF
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW 0xFFE0
#define TFT_WHITE 0xFFFF
#define TFT_COPPER 0xBB86

#define TFT_DC 21  // Data/Command VIDI X zaslona spojen je na PIN 21
#define TFT_CS 5   // Chip select VIDI X zaslona spojen je na PIN 5

#define speed 95  // Less is Fasster

const int btn_uid = 35;  // gumb gore dolje
const int BTN_A = 33;    //BTN_B Button
#define BTN_LR 34  // GPIO34 za kretanje lijevo-desno

// stvaranje TFT objekta (zaslon)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

int answer = 0;
int score = 0;
int Quest = 0;

int PlayerNumber = 0;
unsigned long startTime, endTime;   // Start time, End time for the quiz
unsigned long elapsed;
int Your_Time_Minutes;    // Minutes of elapsed time
int Your_Time_Seconds;    // Seconds of elapsed time

// Globalne varijable za upravljanje unosom imena
char Player_Name[21] = "One";  // Array to store player's name, max 20 characters + null terminator
int nameIndex = 0;          // Current index in the Player_Name array
int letterIndex = 0;        // Current index of the selected letter
int rowIndex = 0;           // Current row index for letter selection

const char* keyboard[4][10] = {
    // Prvi red s brojevima i nekim cesto koristenim simbolima
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"}, // Drugi red s QWERTY rasporedom slova
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"}, // Treci red s QWERTY rasporedom slova
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "<"}, // '<' moze predstavljati Backspace
    {"Z", "X", "C", "V", "B", "N", "M", "-", "_", " "} // cetvrti red s QWERTY rasporedom slova i dodatnim simbolima
};

const int debounceTime = 150; // Adjust this value as needed (in milliseconds)
unsigned long lastButtonPress = 0; // Store the last button press time

bool waitingForNext = false;
bool showTopic = false;
int topicIndex = 0;
const char* topics[] = {"FILMOVI I    SERIJE", "SPORT", "GLAZBA", "HRVATSKI I   MATEMATIKA", "POVIJEST I   GEOGRAFIJA", "INFORMATIKA"};

struct Question {
    String text;                    // The question text
    String options[4];              // The answer options (maximum of 4)
    int correctOption;              // Index of the correct answer (0-based)
    int optionCount;                // Number of options (3 or 4)
};

// Centralized array of questions
#ifdef Player_color_red
    Question questions[] = {
        // FILMOVI I SERIJE
        {"Kako se zove serijal u    kojem su likovi Luke      Skywalker i Darth Vader?", {"The Avengers", "Ratovi Zvijezda", "Zvjezdane staze"}, 1, 3},
        {"Koji film prikazuje borbu protiv umjetne            inteligencije Skynet?", {"Blade Runner", "Avatar", "Terminator"}, 2, 3},
        {"Koji film opisuje         stvaranje atomske bombe?", {"Poor Things", "Oppenheimer", "War is Over!"}, 1, 3},
        {"Tko je osvojio Oscara za  glavnu musku ulogu 2025?", {"Timothee Chalamet", "Adrien Brody", "Ralph Fiennes"}, 1, 3},

        // SPORT
        {"Koji sport igra Lionel    Messi?", {"Kosarka", "Nogomet", "Tenis"}, 1, 3},
        {"Koji sport se igra na     ledu s palicama i pakom?", {"Curling", "Hokej", "Klizanje"}, 1, 3},
        {"U kojem sportu je cilj    ostvariti sto krace       vrijeme?", {"Gimnastika", "Dizanje utega", "Veslanje"}, 2, 3},
        {"Koliko je olimpijskih     medalja osvojila Janica   Kostelic?", {"11", "6", "3"}, 1, 3},

        // GLAZBA
        {"Koji grunge bend je poznat po hitu 'Smells Like Teen Spirit'?", {"Green Day", "Nirvana", "Metallica"}, 1, 3},
        {"Poznata K-Pop grupa zove  se:", {"TBD", "BTS", "BYD"}, 1, 3},
        {"Koji pjevac je            predstavljao Hrvatsku na  Eurosongu ove godine?", {"Marko Perkovic", "Marko Pursic", "Marko Bosnjak"}, 2, 3},
        {"U notnom zapisu G-kljuc   se zove jos i:", {"Klavirski kljuc", "Violinski kljuc", "Truba kljuc"}, 1, 3},

        // HRVATSKI I MATEMATIKA
        {"Kako se zove knjizevna    vrsta u kojoj se pise o   stvarnim dogadajima i     osobama?", {"Poezija", "Publicistika", "Bajka"}, 1, 3},
        {"6 + 2 x (3 + 1) : 2 - 6 =", {"1", "6", "4"}, 2, 3},
        {"Koji je padez u recenici  'Idem u skolu'?", {"Nominativ", "Akuzativ", "Vokativ"}, 1, 3},
        {"Kako se zove jednadzba    oblika ax + b = 0", {"Kvadratna", "Eksponencijalna", "Linearna"}, 2, 3},

        // POVIJEST I GEOGRAFIJA
        {"Koje je godine zavrsio    Prvi svjetski rat?", {"1914.", "1920.", "1918."}, 2, 3},
        {"Koji je glavni grad       Kanade?", {"Toronto", "Montreal", "Ottawa"}, 2, 3},
        {"Tko je bio prvi hrvatski  kralj?", {"Zvonimir", "Tomislav", "Petar Kresimir"}, 1, 3},
        {"Koje se tri rijeke        spominju u hrvatskoj himni 'Lijepa nasa domovino'?", {"Dunav, Sava, Kupa", "Drava, Sava, Dunav", "Sava, Zrmanja, Dunav"}, 1, 3},

        // INFORMATIKA
        {"Sto je 'phishing'?", {"Igra na internetu", "Prevara radi kradje       podataka", "Vrsta racunala"}, 1, 3},
        {"Sto oznacuje skracenica   CPU?", {"Cijeli pogonski uredaj", "Centralna procesorska     jedinica", "Ciklicka programska       unija"}, 1, 3},
        {"Koja se jedinica koristi  za velicinu memorije?", {"Milimetar", "Kilogram", "Gigabyte"}, 2, 3},
        {"Kakav ekran ima VIDI X?", {"crno bijeli", "nema ekran", "touch screen u boji"}, 2, 3},
        {"Kakav ekran ima VIDI X?", {"crno bijeli", "nema ekran", "touch screen u boji"}, 2, 3},
        {"Kakav ekran ima VIDI X?", {"crno bijeli", "nema ekran", "touch screen u boji"}, 2, 3}
    };
#elif defined(Player_color_green)
    Question questions[] = {
        // FILMOVI I SERIJE
        {"U kojoj seriji su likovi  Rachel, Joey, Phoebe,     Chandler, Monica i Ross?", {"Prijatelji", "Susjedi", "Osvetnici"}, 0, 3},
        {"U kojem filmu glavni      negativac nosi crnu kacigu i koristi 'silu'?", {"Matrix", "Ratovi Zvijezda", "Superman"}, 1, 3},
        {"Kako se zove serija sa    zmajevima u zemlji        Westerosa?", {"Witcher", "Igra prijestolja", "Vikinzi"}, 1, 3},
        {"Koji od ovih filmova NIJE znanstveno fantasticni:", {"Titanic", "Dina", "Mickey 17"}, 0, 3},

        // SPORT
        {"U kojem se sportu koristi reket i loptica?", {"Golf", "Baseball", "Tenis"}, 2, 3},
        {"U kojem sportu            natjecatelji bacaju       koplje?", {"Atletika", "Ragbi", "Karate"}, 0, 3},
        {"Koji sport ukljucuje      voznju po stazi velikim   brzinama u formulama?", {"Motokros", "Formula 1", "Rally"}, 1, 3},
        {"Koliko su olimpijskih     medalja osvojila braca    Sinkovic?", {"6", "4", "8"}, 1, 3},

        // GLAZBA
        {"Tko je bio pjevac grupe   Queen?", {"Freddie Mercury", "Donald Trump", "Kanye"}, 0, 3},
        {"Poznati hit Bruna Marsa i ROSE je:", {"BTW.", "APT.", "FYI."}, 1, 3},
        {"Koji izvodac je           predstavljao Hrvatsku na  Eurosongu 2024?", {"Damir Urban", "Baby Lasagna", "Miso Kovac"}, 1, 3},
        {"Prva hrvatska opera       skladatelja Vatroslava    Lisinskog, zove se:", {"Ljubav i zloba", "Ljubav i mrznja", "Moja domovina"}, 0, 3},

        // HRVATSKI I MATEMATIKA
        {"Koje je stilsko izrazajno sredstvo u recenici: 'More je saptalo svoje tajne'?", {"Personifikacija", "Usporedba", "Metafora"}, 0, 3},
        {"5 + 3 x (2 + 4) : 3 - 2 =", {"10", "11", "9"}, 2, 3},
        {"Koja je vrsta rijeci      'brzo' u recenici 'Ivan   brzo trci'?", {"Prilog", "Imenica", "Glagol"}, 0, 3},
        {"Koji geometrijski lik ima sve tocke jednako udaljene od sredista?", {"Kruznica", "Kvadrat", "Trokut"}, 0, 3},

        // POVIJEST I GEOGRAFIJA
        {"Tko je bio britanski      premijer u Drugom         svjetskom ratu?", {"Winston Churchill", "Franklin Roosevelt", "Harry Truman"}, 0, 3},
        {"Koji hrvatski planinski   lanac pripada Dinaridima?", {"Velebit", "Papuk", "Medvednica"}, 0, 3},
        {"U kojem gradu se nalazi   Dioklecijanova palaca?", {"Split", "Dubrovnik", "Pula"}, 0, 3},
        {"Koja tri grada su bili    prijestolnice Hrvatskog   Kraljevstva?", {"Knin, Zagreb, Varazdin", "Nin, Biograd, Knin", "Zagreb, Osijek, Split"}, 1, 3},

        // INFORMATIKA
        {"Sto je dvofaktorska       autentifikacija?", {"Dvije razine provjere     identiteta", "Dvije lozinke iste        vrste", "Dva korisnicka imena"}, 0, 3},
        {"Koji od navedenih dijelova je ulazna jedinica?", {"Tipkovnica", "Zvucnik", "Monitor"}, 0, 3},
        {"Kojom se jedinicom        oznacava brzina procesora?", {"gigahertz (GHz)", "kilometar (km)", "gigybyte (GB)"}, 0, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3}
    };
#elif defined(Player_color_blue)
    Question questions[] = {
        // FILMOVI I SERIJE
        {"U kojem filmu svemirac    lovi specijalce po        prasumi?", {"Predator", "Mandalorian", "Spiderman"}, 0, 3},
        {"Koji film prati grupu     superheroja poput Iron    Mana i Thora?", {"Fantastic Four", "The Avengers", "Asterix"}, 1, 3},
        {"Kako se zove animirani    film o dobrocudnom        zelenom ogru?", {"Shrek", "Ledeno doba", "Aladin"}, 0, 3},
        {"Koji hrvatski film je bio nominiran za Oscara?", {"Drazen", "Covjek koji nije mogao    sutjeti", "Zecji nasip"}, 1, 3},

        // SPORT
        {"Navijacka skupina         nogometnog kluba Dinamo   zove se:", {"Bad Blue Boys", "Babaroge", "Buldozi"}, 0, 3},
        {"Koji sportas je osvojio 4 srebrne olimpijske        medalje?", {"Ivica Kostelic", "Luka Modric", "Toni Kukoc"}, 0, 3},
        {"Koji sport se igra na     parketu s kosem na oba    kraja?", {"Kosarka", "Rukomet", "Odbojka"}, 0, 3},
        {"Tour de France je utrka:", {"Biciklisticka", "Trkacka", "Plivacka"}, 0, 3},

        // GLAZBA
        {"Grupa s najvise prodanih  ploca na svijetu je:", {"The Beatles", "Rolling Stones", "Pearl Jam"}, 0, 3},
        {"Koje grcko slovo je u     naslovu najpopularnijeg   TikTok hita?", {"Delta Boy", "Sigma Boy", "Kappa Boy"}, 1, 3},
        {"Koja pjevacica je odrzala rasprodanu turneju 'Eras'?", {"Taylor Swift", "Katy Perry", "Lady Gaga"}, 0, 3},
        {"Za koji glazbeni sastav je Beethoven pisao 9.       simfoniju?", {"Klasicni orkestar", "Jazz orkestar", "Rock band"}, 0, 3},

        // HRVATSKI I MATEMATIKA
        {"Koji je knjizevnik napisao roman 'U registraturi'?", {"Ante Kovacic", "August Senoa", "Ksaver Sandor Gjalski"}, 0, 3},
        {"8 - 2 x (5 - 3) : 2 - 6 =", {"0", "2", "6"}, 0, 3},
        {"Kako se zove znanost o    jeziku?", {"Lingvistika", "Hrvatski jezik", "Lektira"}, 0, 3},
        {"Koja je mjerna jedinica za povrsinu kruga?", {"Kvadratni metar", "Metar", "Kvadratni krug"}, 0, 3},

        // POVIJEST I GEOGRAFIJA
        {"Hrvatski Dan drzavnosti se slavi:", {"30. svibnja", "22. lipnja", "15. kolovoza"}, 0, 3},
        {"Koja rijeka protjece kroz grad Osijek?", {"Drava", "Sava", "Dunav"}, 0, 3},
        {"Koja je bila prijestolnica Kraljevine Hrvatske u    srednjem vijeku?", {"Zagreb", "Knin", "Osijek"}, 1, 3},
        {"Koje se tri zivotinje     nalaze u grbu Republike   Hrvatske?", {"Kuna, Slavuj, Medvjed", "Kuna, Koza, Lav", "Koza, Kuna, Medvjed"}, 1, 3},

        // INFORMATIKA
        {"Sto je 'firewall'?", {"Zastita racunala od       napada", "Vrsta internetske veze", "Program za obradu         teksta"}, 0, 3},
        {"Koji od navedenih uredaja omogucuje spajanje na     internet?", {"Zvucnik", "Printer", "Wi-Fi modul"}, 2, 3},
        {"Koja je kratica za 'Radna Memorija'?", {"RAM", "ROM", "SSD"}, 0, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3},
        {"VIDI X se moze            programirati u jeziku:", {"Samo C++", "VisualBasic", "C, C++, MicroPython,      Scratch..."}, 2, 3}
    };
#endif

String getCurrentQuestionHTML() {
  String primaryColor;

  // Set colors based on the player's color
  if (Player_color == "red") {
    primaryColor = "red";
  } else if (Player_color == "green") {
    primaryColor = "green";
  } else if (Player_color == "blue") {
    primaryColor = "blue";
  }

  String html = R"rawliteral(
  <!DOCTYPE html>
  <html><head>
  <title>VIDI X Quiz</title>
  <meta charset="utf-8">
  <style>
    body {
        background-color: black; /* Crna pozadina */
        color: white; /* Bijeli tekst prema zadanim postavkama */
        font-family: Arial, sans-serif; /* Lijep, moderan font */
        margin: 20;
        padding: 0px;
    }
    h1 {
        font-size: 6rem; /* Najveci tekst za naslov */
        color: yellow; /* zuti tekst */
        background-color: )rawliteral" + primaryColor + R"rawliteral(; /* Dinamicna pozadina odgovora */
        text-align: center; /* Centriranje teksta */
        padding: 20px; /* Dodavanje razmaka */
        margin-bottom: 40px; /* Odmak ispod naslova */
        white-space: nowrap; /* Sprjecava lomljenje rijeci u naslovu */
    }
    #quest {
        font-size: 3rem; /* Prilagodena velicina teksta */
        color: white; /* Bijeli tekst */
        margin-bottom: 20px; /* Razmak ispod pitanja */
        background-color: )rawliteral" + primaryColor + R"rawliteral(; /* Dinamicna pozadina odgovora */
    }
    #quest::after {
        content: "--------------------------------------------"; /* Linie ispod pitanja */
        display: block;
        text-align: center; /* Centriranje teksta */
        font-size: 3rem;
        color: white; /* Bijeli tekst */
        margin-top: 10px;
    }
    #answers p {
        font-size: 3rem; /* Velicina teksta za odgovore */
        margin: 10px 0; /* Razmak izmedu odgovora */
        background-color: )rawliteral" + primaryColor + R"rawliteral(; /* Dinamicna pozadina odgovora */
    }
    #score {
        font-size: 3rem; /* Velicina teksta za rezultat */
        color: white; /* Bijeli tekst */
        background-color: )rawliteral" + primaryColor + R"rawliteral(; /* Dinamicna pozadina rezultata */
        text-align: center; /* Centriranje rezultata */
        padding: 10px; /* Dodavanje razmaka */
        margin-top: 40px; /* Odmak iznad rezultata */
        white-space: nowrap; /* Sprjecava lomljenje rijeci */
    }
  </style>

  <script>
    function loadQuestion() {
        fetch('/api')
            .then(response => {
                if (!response.ok) {
                    throw new Error("Network response was not ok");
                }
                return response.json();
            })
            .then(data => {
                if (data.topic) {
                    document.getElementById("quest").innerHTML = '<div style="text-align: center; font-size: 200%; font-weight: bold;">TEMA: ' + data.topic + '</div>';

                    document.getElementById("answers").innerHTML = ""; // Clear answers
                    document.getElementById("score").innerHTML = ""; // Clear score
                } else {
                    document.getElementById("quest").innerHTML = "PITANJE " + data.quest + ": " + data.question;
                    let html = "";
                    for (let i = 0; i < data.options.length; i++) {
                        html += "<p>Odgovor " + String.fromCharCode(65 + i) + ": " + data.options[i] + "</p>";
                    }
                    document.getElementById("answers").innerHTML = html;
                    document.getElementById("score").innerHTML = "BODOVI: " + data.score;
                }
            })
            .catch(error => {
                console.error("There was a problem with the fetch operation:", error);
            });
    }

    // Automatically reload the question every 3 seconds
    setInterval(loadQuestion, 3000);

    window.onload = loadQuestion;
  </script>

  </head><body>
    <h1>VIDI X Quiz</h1>
    <p id="quest">Ucitavanje pitanja...</p>
    <div id="answers"></div>
    <p id="score">Ucitavanje rezultata...</p>
  </body></html>
  )rawliteral";

  return html;
}

void waitForRightPress(int pressesRequired = 1, bool showMessage = false) {
    int pressCount = 0;
    int flag = 0;
    while (pressCount < pressesRequired) {
        if (showMessage && pressCount == 0) {
          if (flag == 0 ) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_YELLOW);
            tft.setTextSize(3);
            flag = 1;
            }
            tft.setCursor(0, 100);
            tft.println("-----------------");
            tft.println("DESNO za nastavak");
            tft.println("-----------------");
        }
        //if (digitalRead(BTN_LR) == LOW) {  // Wait for "deno" button press  if ( v > 1700 && v < 2100 )
        if (analogRead(BTN_LR) != 4095 ) {
          if (analogRead(BTN_LR) > 1500 ) {  // Wait for "deno" button press
          
              delay(200);  // Debounce delay
              pressCount++;
              //while (digitalRead(BTN_LR) == LOW);  // Wait for button release
              while (analogRead(BTN_LR) > 1500);  // Wait for button release
          }
        }
    }
    showTopic = false;  // Exit topic screen
}

void showTopicScreen() {
  cls();
  Screen_Header();
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.setCursor(20, 80);
  tft.println("TEMA:");
  tft.setTextSize(4);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(20, 130);
  tft.println(topics[topicIndex % 6]);  // Display the topic on the screen
  showTopic = true;  // Indicate that the topic screen is active
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_READ, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Postavljanje pina gumba kao ulaz

  PlayerNumber++;  // Increment player number for new session

  // definiramo input pinove tj. upravljacke gumbe
  pinMode(BTN_A, INPUT_PULLUP);  // A gumb mora biti definiran kao INPUT_PULLUP
  pinMode(btn_uid, INPUT_PULLUP);
  //pinMode(BTN_LR, INPUT_PULLUP);
  //pinMode(btn_uid, INPUT);  // Postavljeno kao ulaz bez pull-upa jer koristimo analogRead
  pinMode(BTN_LR, INPUT);   // Isto za lijevo/desno

  // inicijalizacija zaslona
  tft.begin();
  // postavljamo orijentaciju ekzaslonarana
  tft.setRotation(3);
  // definiramo boju pozadine
  tft.fillScreen(ILI9341_BLACK);
  // delay(2000);

  //enterName();
Quest++; //Treba samo ako je enterName(); zakomentiran u retku iznad

    // Start time measuring
  startTime = millis();
  
  strip.begin();
  strip.show(); // Iskljuci sve LED-ice na pocetku
  ledcSetup(0, 2000, 8); // Kanal 0, frekvencija 2kHz, rezolucija 8-bit
  ledcAttachPin(SPEAKER_PIN, 0);

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  tft.println("Access Point started.");
  tft.print("SSID: "); Serial.println(ssid);
  tft.print("Visit this IP in browser: ");
  tft.println(WiFi.softAPIP());  // obicno 192.168.4.1

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("answer")) {
      answer = request->getParam("answer")->value().toInt();
      go_OK();  // simulira pritisak gumba OK
    }
    request->send(200, "text/html", getCurrentQuestionHTML());
  });

server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    String json;
    if (showTopic) {
        json = "{\"topic\":\"" + String(topics[(topicIndex - 1 + 6) % 6]) + "\"}"; // Adjust topic index for web page
    } else {
        json = "{\"quest\":" + String(Quest) + ",";
        json += "\"question\":\"" + questions[Quest - 1].text + "\",";
        json += "\"options\":[";
        for (int i = 0; i < questions[Quest - 1].optionCount; i++) {
            if (i > 0) json += ",";
            json += "\"" + questions[Quest - 1].options[i] + "\"";
        }
        json += "],";
        json += "\"score\":" + String(score) + "}";
    }
    request->send(200, "application/json", json);
});

  server.on("/answer", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
      answer = request->getParam("val")->value().toInt();
      go_OK();  // Odmah evaluiraj
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();

  showTopicScreen();  // Show the first topic screen at the start
  topicIndex++;
  waitForRightPress(2);  // Require two presses to proceed

  cls();
}

void displayQuestion(int questionIndex) {
    const Question& q = questions[questionIndex];
    tft.setCursor(0, 0);
    Screen_Header();
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.println(q.text);
    tft.println("__________________________");
    for (int i = 0; i < q.optionCount; i++) {
        if (answer == (i + 1)) {
            tft.setTextColor(TFT_RED, TFT_YELLOW);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLUE);
        }
        tft.println(String((char)('A' + i)) + ") " + q.options[i]);
    }
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
}

void playPinballSound() {
  // Pinball efekt - uzlazna frekvencija
  for (int i = 500; i < 1500; i += 50) {
    ledcWriteTone(0, i);
    delay(20);
  }
  ledcWriteTone(0, 0); // Stop sound
}

void lightShow() {
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < LED_COUNT; i++) {
      uint32_t color = strip.Color(0, 0, 0);  // default off

      if (Player_color == "red")      color = strip.Color(255, 0, 0);    //Raspored boja je Green Red Blue - NEO_GRB
      else if (Player_color == "blue") color = strip.Color(0, 255, 0);  //Raspored boja je Green Red Blue - NEO_GRB
      else if (Player_color == "green")  color = strip.Color(0, 0, 255);  //Raspored boja je Green Red Blue - NEO_GRB

      strip.setPixelColor(i, color);

    //      Serial.print("Player_color ");
    //Serial.println(Player_color);
    //Serial.print("color ");
    //Serial.println(color);

      strip.show();
      delay(80);
      strip.setPixelColor(i, 0);
    }
  }
}

void showScoreLEDs() {
  strip.clear();
  for (int i = 0; i < min(score, LED_COUNT); i++) {
    uint32_t color = strip.Color(0, 0, 0);

    if (Player_color == "red")      color = strip.Color(255, 0, 0);    //Raspored boja je Green Red Blue - NEO_GRB
    else if (Player_color == "blue") color = strip.Color(0, 255, 0);  //Raspored boja je Green Red Blue - NEO_GRB
    else if (Player_color == "green")  color = strip.Color(0, 0, 255);  //Raspored boja je Green Red Blue - NEO_GRB

    strip.setPixelColor(i, color);
    //Serial.print("Player_color ");
    //Serial.println(Player_color);
    //Serial.print("color ");
    //Serial.println(color);

  }
  strip.show();
}

void drawKeyboard(int activeRow, int activeCol) {
    //tft.fillScreen(TFT_BLACK);  // Clear the screen
    Screen_Header();
    tft.setTextSize(2);         // Set text size for keyboard display
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.println("Move - UP DOWN LEFT RIGH");
    tft.println("Select  - BTN_A");
    tft.println("Confirm - START");
    int startX = 10;
    int startY = 90;
    int lineHeight = 30;
    int colWidth = 30;
    tft.setTextSize(3);
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 10; col++) {
            //tft.drawRect(startX + activeCol * colWidth - 2, startY + activeRow * lineHeight - 2, colWidth/2 , lineHeight/2 + 2, TFT_BLUE);  // Add blue border
            if (row == activeRow && col == activeCol) {
                tft.setTextColor(TFT_RED, TFT_YELLOW);  // Active letter color
                //tft.drawRect(startX + activeCol * colWidth - 2, startY + activeRow * lineHeight - 2, colWidth/2 , lineHeight/2 + 2, TFT_YELLOW);  // Add yellow border
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLUE);  // Other letters color
            }
            tft.setCursor(startX + col * colWidth, startY + row * lineHeight);
            tft.print(keyboard[row][col]);
        }
    }
    tft.setTextSize(2); 
}

void enterName() {
    int currentRow = 0, currentCol = 0;  // Start from the top-left corner of the keyboard
    drawKeyboard(currentRow, currentCol);  // Initial draw of the keyboard
    updateNameDisplay();

    while (true) {
        int lrValue = analogRead(BTN_LR);
        if (lrValue > 4000) {  // Move left
            delay(speed);  // Debounce delay
            currentCol = (currentCol + 9) % 10;  // Wrap around to the left
            drawKeyboard(currentRow, currentCol);
        }
        if (lrValue > 1800 && lrValue < 2200) {  // Move right
            delay(speed);
            currentCol = (currentCol + 1) % 10;  // Wrap around to the right
            drawKeyboard(currentRow, currentCol);
        }

        int udValue = analogRead(btn_uid);
        if (udValue > 4000) {  // Move up
            delay(speed);
            currentRow = (currentRow + 3) % 4;  // Wrap around upwards
            drawKeyboard(currentRow, currentCol);
        }
        if (udValue > 1800 && udValue < 2200) {  // Move down
            delay(speed);
            currentRow = (currentRow + 1) % 4;  // Wrap around downwards
            drawKeyboard(currentRow, currentCol);
        }

        if (analogRead(BTN_A) == 0) {  // Select letter
            delay(3 * speed);  // Debounce delay
            if (strcmp(keyboard[currentRow][currentCol], "<") == 0) {  // Check if backspace was selected
                if (nameIndex > 0) {
                    Player_Name[--nameIndex] = '\0';  // Remove last character
                }
            } else if (nameIndex < sizeof(Player_Name) - 1) {
                Player_Name[nameIndex++] = *keyboard[currentRow][currentCol];  // Add character to name
                Player_Name[nameIndex] = '\0';  // Null-terminate string
            }
            updateNameDisplay();  // Update display with new name
        }

        if (nameIndex == 3 || ((digitalRead(BUTTON_PIN) == LOW) && (nameIndex > 0))) {  // Exit name entry after 3 characters or on button press

            delay(9 * speed);
            countdown_321();
            tft.fillScreen(TFT_BLACK);
            Quest++;
            break;
        }
    }
}

void updateNameDisplay() {
    //tft.fillRect(0, 150, 320, 40, TFT_BLACK);  // Clear the area for the name display
    tft.setTextColor(TFT_YELLOW, TFT_RED);
    tft.setCursor(0, 210);
    tft.print("Choose a 3-character      Nickname: ");
    tft.print(Player_Name);  // Display the current name
    tft.print(" ");
}

void countdown_321() {
    tft.fillScreen(TFT_BLACK);
    Screen_Header();  // Prikazuje zaglavlje koje ostaje vidljivo

    int centerX = 160;  // Centar zaslona po X osi (ovisno o dimenzijama vaseg zaslona)
    int centerY = 120;  // Centar zaslona po Y osi
    int boxWidth = 240; // sirina pravokutnika za ciscenje
    int boxHeight = 160; // Visina pravokutnika za ciscenje

    // Brojevi za odbrojavanje i njihove pocetne velicine fonta
    int numbers[] = {3, 2, 1};
    int startSize = 25;  // Pocetna velicina fonta (vrlo velika)

    for (int i = 0; i < 3; i++) {
        for (int size = startSize; size > 6; size -= 1) {
            //tft.fillRect(centerX - boxWidth/2, centerY - boxHeight/2, boxWidth, boxHeight, TFT_BLACK);  // Ocisti samo sredinu zaslona
            tft.setCursor(centerX - size*3, centerY - size*3);  // Ajustiraj poziciju kursora ako je potrebno
            tft.setTextSize(size);  // Postavi velicinu teksta
            tft.setTextColor(TFT_RED, TFT_BLACK);  // Postavi boju brojeva
            tft.print(numbers[i]);  // Ispisi broj
            delay(10);  // Pauza za vizualni efekt smanjivanja
            tft.setCursor(centerX - size*3, centerY - size*3);  // Ajustiraj poziciju kursora ako je potrebno
            tft.setTextSize(size);  // Postavi velicinu teksta
            tft.setTextColor(TFT_BLACK, TFT_BLACK);  // Postavi boju brojeva
            tft.print(numbers[i]);  // Ispisi broj
        }
        delay(700);  // Pauza prije prikazivanja sljedeceg broja
    }

    // Ispis "GO!" na kraju odbrojavanja
    tft.fillRect(centerX - boxWidth/2, centerY - boxHeight/2, boxWidth, boxHeight, TFT_BLACK);  // Ocisti sredinu zaslona
    tft.setCursor(centerX - 60, centerY - 30);  // Centriraj "GO!" (prilagodi po potrebi)
    tft.setTextSize(8);  // Velika velicina za "GO!"
    tft.setTextColor(TFT_GREEN, TFT_BLACK);  // Zelena boja za "GO!"
    tft.print("GO!");
    delay(1000);  // Prikazuje "GO!" 1 sekundu
}


void go_down() {
  answer++;
  if (answer > 3) {
    answer = 3;
  }
}

void go_up() {
  answer--;
  if (answer < 1) {
    answer = 1;
  }
}

void go_OK() {
    if (showTopic) {
        showTopic = false;  // Exit topic screen
        cls();
        return;
    }

    if (answer - 1 == questions[Quest - 1].correctOption) {
        score++;
        Screen_OK_Answer();
    } else {
        Screen_Wrong_Answer();
    }
    waitForRightPress(1, true);  // Wait for "deno" button press with message
    cls();
    answer = 0;
    Quest++;
    if (Quest == 25 ) {
      End_Screen();
    }
    if (Quest % 4 == 1) {  // Show topic screen after every 4th question
        showTopicScreen();
        topicIndex++;
        waitForRightPress(2);  // Require two presses to proceed
        cls();  // Clear the screen before showing the next question
    }
}

void Screen_Header() {
  tft.setTextColor(TFT_YELLOW, TFT_RED);
  tft.setTextSize(4);
  tft.setCursor(0, 0);
  tft.println(" VIDI X QUIZ ");
}

void End_Screen() {
  //tft.fillScreen(ILI9341_BLACK);
  Screen_Header();

  tft.setTextColor(TFT_RED);
  tft.setTextSize(3);

  if (score > 0) {
    tft.println("=================");
    tft.println("Cestitamo!");
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    //tft.printf("Player %d: %s\n", PlayerNumber, Player_Name);
    //tft.printf("Time %lu ms\n", endTime - startTime);
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED);
    tft.println("=================");
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    //tft.println("  ");
    tft.print("Imas ");
    tft.print(score);
    if (score == 1) {
      tft.println(" tocan odgovor");
    } else if (score > 1 && score < 5 ) {
      tft.println(" tocna odgovora");
    } else {
      tft.println(" tocnih odgovora");
    }
    tft.println(" do sada.");
  } else {
    tft.println("*===============*");
    tft.println(" Vise srece");
    tft.println("      drugi puta!");
    tft.println("*===============*");
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.println("  ");
    //tft.println("Your don't have");
    //tft.println("     correct answers!");
  }

  while (true) {
    showScoreLEDs();
    delay(520);
    uint32_t color = strip.Color(0, 0, 0);  // default off
    for (int i = 0; i < min(score, LED_COUNT); i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
    delay(180);
  }

  //Display time in minutes and seconds
  //tft.println("Your time is:");
  //tft.print(Your_Time_Minutes); 
  //tft.print(" min. and ");
  //tft.print(Your_Time_Seconds);
  //tft.print(" s.");
  delay(50000000);
  cls();
}

void cls() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
}

void Screen_OK_Answer() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Tvoj odgovor je");
  tft.println("TOCAN !!!");

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.println("  ");
  tft.print("Imas ");
  tft.print(score);
  if (score == 1) {
    tft.println(" tocan odgovor");
  } else if (score > 1 && score < 5 ) {
    tft.println(" tocna odgovora");
  } else {
    tft.println(" tocnih odgovora");
  }
  tft.println(" do sada.");

  playCorrectAnswerAnimation();  // Updated animation for correct answer
  showScoreLEDs();               // Display score on LEDs

  delay(3000);
  cls();
}

void Screen_Wrong_Answer() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Tvoj odgovor");
  tft.setTextColor(TFT_RED);
  tft.print("NIJE ");
  tft.setTextColor(TFT_CYAN);
  tft.println("tocan !!!");

  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.println("  ");
  tft.print("Imas ");
  tft.print(score);
  if (score == 1) {
    tft.println(" tocan odgovor");
  } else if (score > 1 && score < 5 ) {
    tft.println(" tocna odgovora");
  } else {
    tft.println(" tocnih odgovora");
  }
  tft.println(" do sada.");

  playIncorrectAnswerAnimation();  // Updated animation for incorrect answer
  showScoreLEDs();                 // Display score on LEDs

  delay(3000);
  cls();
}

void playCorrectAnswerAnimation() {
    uint32_t mainColor = 0, dimColor = 0;

    // Determine the player's color
    if (Player_color == "red") {
        mainColor = strip.Color(255, 0, 0);  // Full intensity red
        dimColor = strip.Color(128, 0, 0);  // 50% intensity red
    } else if (Player_color == "blue") {
        mainColor = strip.Color(0, 255, 0);  // Full intensity green
        dimColor = strip.Color(0, 128, 0);  // 50% intensity green
    } else if (Player_color == "green") {
        mainColor = strip.Color(0, 0, 255);  // Full intensity blue
        dimColor = strip.Color(0, 0, 128);  // 50% intensity blue
    }

    for (int pass = 0; pass < 2; pass++) {
        // Forward animation
        for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, mainColor);  // Middle LED full intensity
            if (i > 0) strip.setPixelColor(i - 1, dimColor);  // Previous LED 50% intensity
            if (i > 1) strip.setPixelColor(i - 2, 0);  // Turn off LED before the previous
            strip.show();
            delay(30);  // Faster animation
        }
        // Backward animation
        for (int i = LED_COUNT - 1; i >= 0; i--) {
            strip.setPixelColor(i, mainColor);  // Middle LED full intensity
            if (i < LED_COUNT - 1) strip.setPixelColor(i + 1, dimColor);  // Next LED 50% intensity
            if (i < LED_COUNT - 2) strip.setPixelColor(i + 2, 0);  // Turn off LED after the next
            strip.show();
            delay(30);  // Faster animation
        }
    }

    // Blink the entire strip 3 times
    for (int i = 0; i < 3; i++) {
        strip.fill(mainColor);  // Full intensity
        strip.show();
        delay(150);
        strip.clear();
        strip.show();
        delay(150);
    }
}

void playIncorrectAnswerAnimation() {
    // Blink the entire strip purple 3 times
    for (int i = 0; i < 3; i++) {
        strip.fill(strip.Color(128, 128, 0));  // Purple
        strip.show();
        delay(150);
        strip.clear();
        strip.show();
        delay(150);
    }

    // Gradually turn off LEDs from the last to the first
    for (int i = LED_COUNT - 1; i >= 0; i--) {
        strip.setPixelColor(i, 0);  // Turn off LED
        strip.show();
        delay(80);  // Slow fade-out
    }
}

void loop() {
    // Provjera stanja gumba za gore, dolje i potvrdu
    int buttonState = analogRead(btn_uid); // citanje analognog stanja gumba za gore/dolje

    if (buttonState > 4000 && millis() - lastButtonPress > debounceTime) {
        // Gumb za gore je pritisnut
        go_up();
        lastButtonPress = millis(); // Azuriraj vrijeme zadnjeg pritiska gumba
    } else if (buttonState > 1800 && buttonState < 2200 && millis() - lastButtonPress > debounceTime) {
        // Gumb za dolje je pritisnut
        go_down();
        lastButtonPress = millis(); // Azuriraj vrijeme zadnjeg pritiska gumba
    }
    if (answer != 0) {
        if (analogRead(BTN_A) == 0 && millis() - lastButtonPress > debounceTime) {
            // Gumb za potvrdu (OK) je pritisnut
            //Serial.print("odgovor ");
            //Serial.println(answer);
            go_OK();
            lastButtonPress = millis(); // Azuriraj vrijeme zadnjeg pritiska gumba
        }
    }
    // Prikaz trenutnog pitanja
    //Serial.print("Quest ");
    //Serial.print(Quest);
        //Serial.print(" sizeof(questions) / sizeof(questions[0]) ");
    //Serial.println( sizeof(questions) / sizeof(questions[0]) );
    if (Quest > 0 && Quest <= sizeof(questions) / sizeof(questions[0])) {
       //Serial.print("PITANJE ");
        displayQuestion(Quest - 1);
    } else {
      //Serial.print("END ");
        End_Screen();
    }
}
