# VIDI X BRAIN BATTLE - Wi-Fi kviz znanja

Ova aplikacija omogućuje učenicima da koriste VIDI X mikroračunalo kao Wi-Fi pristupnu točku putem koje mogu igrati kviz znanja u web pregledniku. Svaki tim (igrač) ima svoju boju (crvenu, plavu ili zelenu), vlastiti set pitanja i bodovni sustav prikazan putem LED trake spojene na GPIO32.

Korištenjem VIDI X kviza na satu informatike moguće je nenametljivo integrirati npr. zemljopisne teme kroz interaktivna pitanja, čime se potiče međupredmetno učenje i veća angažiranost učenika.

Kviz možete pokretati i bez RGB trake ili spajanja na WI-Fi mrežu, te opcije su proizvoljne.

## 📶 Kako se povezati na VIDI X Wi-Fi

1. Uključite VIDI X mikroračunalo.
2. Na svom računalu ili pametnom telefonu potražite Wi-Fi mrežu:
   - **Crveni tim:** `VIDIX1`
   - **Zeleni tim:** `VIDIX2`
   - **Plavi tim:** `VIDIX3`
3. Lozinka za Wi-Fi na VIDI X mikroračunalu je:
   ```
   vidix1234
   ```
4. Kada se povežete, otvorite web preglednik i unesite sljedeću adresu:
   ```
   http://192.168.4.1
   ```

## 🎮 Odabir igrača (tima)

Unutar koda pronađite ovaj dio:

```cpp
// Define the player color
 #define Player_color_red
// #define Player_color_green
// #define Player_color_blue
```

👉 **Ovdje samo jedna od linija smije biti aktivna (bez `//`).**  
Ako želite igrati kao plavi tim, promijenite tako da linija izgleda ovako:

```cpp
// #define Player_color_red
// #define Player_color_green
#define Player_color_blue
```

## ✍️ Kako dodati ili promijeniti pitanja

Pitanja su definirana u strukturi:

```cpp
struct Question {
    String text;            // Tekst pitanja
    String options[4];      // Do 4 ponuđena odgovora
    int correctOption;      // Točan odgovor (0=A, 1=B, 2=C...)
    int optionCount;        // Broj ponuđenih odgovora (obično 3 ili 4)
};
```

Svaki tim ima svoj set pitanja unutar:

```cpp
#ifdef Player_color_red
    Question questions[] = {
        {"Pitanje?", {"Odgovor A", "Odgovor B", "Odgovor C"}, 1, 3},
        ...
    };
#elif defined(Player_color_green)
    ...
#elif defined(Player_color_blue)
    ...
#endif
```

### 🔧 Primjer izmjene pitanja

Ako želite promijeniti pitanje za crveni tim, zamijenite sadržaj unutar `questions[]`:

```cpp
{"Koji je glavni grad Hrvatske?", {"Split", "Zagreb", "Rijeka"}, 1, 3},
```

👉 `1` označava da je **Zagreb** točan odgovor (broji se od 0).

### 🧠 Teme

Svako četvrto pitanje prikazuje novu temu iz niza:

```cpp
const char* topics[] = {
  "FILMOVI I    SERIJE",
  "SPORT",
  "GLAZBA",
  "HRVATSKI I   MATEMATIKA",
  "POVIJEST I   GEOGRAFIJA",
  "INFORMATIKA"
};
```

## 💡 Opće informacije o kvizu

- Pitanja se prikazuju lokalno na VIDI X zaslonu, ali i putem preglednika na tabletu ili laptopu kako bi i publika mogla vidjeti pitanja.
- Nakon svakog odgovora:
  - Točan odgovor animira LED traku u boji tima.
  - Pogrešan odgovor daje ljubičastu animaciju.
- Nakon svaka 4 pitanja prikazuje se nova tema.
- Završni rezultat prikazuje se na zaslonu i LED-icama.
- Moguće je igrati samostalno ili u timovima.

## 📸 Pogledaj kako je izgledalo na terenu!

Timovi **Nikola Tesla**, **Maria Curie** i **Albert Einstein** testirali su ovu aplikaciju na natjecanju VIDI Innovation Day 2025. Pročitaj više:

👉 [VIDI X Brain Battle: kviz znanja oduševio brojne učenike na Innovation Day 2025](https://hr.vidi-x.org/vidi-x-brain-battle-kviz-znanja-odusevio-brojne-ucenike-na-innovation-day-2025/)

![Vidi-X-Innovation-day-2025-18](https://github.com/user-attachments/assets/210cab9b-2d90-49db-a8f7-690befc3c7fe)


---

### 👨‍💻 Podržano mikroračunalo

- VIDI X mikroračunalo s ESP32 procesorom  
- Adafruit_ILI9341 biblioteka  
- Adafruit_NeoPixel (za RGB traku)  
- Korisnički unos preko tipki ugrađenih na VIDI X mikroračunalo

---
