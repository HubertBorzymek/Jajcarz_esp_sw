# Testy manualne

Testy wymagające płytki. Wynik zaznacz `[x]`, a problem opisz pod punktem.

## v0.1.0: panel bez sieci

Przygotowanie: płytka podłączona przez USB, w nowym oknie PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
. C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1
cd C:\Users\leemi\Documents\Projects\Esp32\Jajcarz_sw
idf.py -p COM3 flash monitor
```

Pierwsza linia zezwala na skrypty tylko w tym oknie. Trzeba powtórzyć te polecenia
w każdym nowym oknie. Wyjście z monitora: `Ctrl+]`. Pozostaw monitor otwarty, żeby
widzieć log. Jeśli port jest zajęty, zamknij monitor w VS Code.

### Start

- [x] Po resecie (EN) wszystkie 4 diody świecą kolejno na czerwono, zielono, niebiesko.
- [x] Każdy kolor trwa około 0,3 s.
- [x] Log pokazuje `Jajcarz <wersja>` i `Gotowe, klawisze: 4`.
- [x] Przy pierwszym uruchomieniu po teście wszystkie diody są zielone.

### Przyciski

| Przycisk | Adres w logu |
|---|---|
| SW1 | 0.0 |
| SW2 | 0.1 |
| SW3 | 0.2 |
| SW4 | 0.3 |

- [x] SW1: zielony → żółty → czerwony → zielony, zmienia się tylko dioda 1.
- [x] SW2: to samo, tylko dioda 2.
- [x] SW3: to samo, tylko dioda 3.
- [x] SW4: to samo, tylko dioda 4.
- [x] Każde naciśnięcie daje dokładnie jeden wpis w logu, np. `0.1 -> LOW`.
- [x] Przytrzymanie przez 3 s daje jedną zmianę.
- [x] Szybkie klikanie nie daje podwójnych zmian.
- [x] Przytrzymanie przycisku podczas resetu nie zmienia stanu.
- [x] Żółty nie wpada w zieleń.

### Pamięć stanów

- [x] Ustaw różne kolory i naciśnij EN: po teście startowym kolory wracają.
- [x] Ustaw różne kolory i odłącz USB na 5 s: po podłączeniu kolory wracają.

### Dioda sygnalizacyjna

- [x] Dioda na płytce miga mniej więcej raz na sekundę.

Jeśli nie miga, sprawdź, czy jakakolwiek dioda na płytce reaguje. Wtedy trzeba
zmienić `STATUS_LED_GPIO` w `components/status_led/src/status_led.c`.

Po zaliczeniu tej sekcji: tag `v0.1.0`.

## v0.2.0: połączenie z serwerem

### Przygotowanie

1. Uzupełnij `config/secrets.h`: nazwa i hasło Wi-Fi, adres serwera i token.
   Plik jest poza gitem. Adres to `ws://<IP_komputera>:5080/ws/device`,
   a IP sprawdzisz poleceniem `ipconfig`. Token testowy: `test-token`.
2. Uruchom serwer testowy:
   ```powershell
   dotnet run --project "C:\Users\leemi\Documents\Projects\Visual Studio\Jajcarz_test_server\src\Jajcarz.TestServer"
   ```
3. Otwórz w przeglądarce `http://localhost:5080`.
4. Przy pierwszym uruchomieniu zezwól na dostęp w zaporze Windows.
5. Wgraj firmware: `idf.py -p COM3 flash monitor`.

Komputer i ESP32 muszą być w tej samej sieci, a ESP32 obsługuje tylko 2,4 GHz.

### Połączenie

- [ ] Log pokazuje połączenie z Wi-Fi i serwerem.
- [ ] Dioda na płytce przestaje migać i świeci stale.
- [ ] Panel pojawia się na stronie serwera z 4 klawiszami.
- [ ] Diody na panelu pokazują stany ze strony.

### Panel → serwer

- [ ] Naciśnięcie przycisku zmienia stan na stronie w ciągu około 1 s.
- [ ] Strona pokazuje autora zmiany `device:<id>` i czas.
- [ ] Dioda na płytce krótko gaśnie po potwierdzeniu przez serwer.

### Serwer → panel

- [ ] Zmiana stanu na stronie zmienia diodę w ciągu około 1 s.
- [ ] Strona pokazuje autora `web:test`.
- [ ] Odznaczenie „przypisany” gasi diodę klawisza.
- [ ] Naciśnięcie nieprzypisanego klawisza nie zmienia stanu, a strona pokazuje je w dzienniku.
- [ ] Ponowne przypisanie zapala diodę.
- [ ] Przycisk „Identyfikuj” miga wybraną diodą przez około 3 s, także nieprzypisaną.

### Utrata połączenia

- [ ] Zatrzymanie serwera: dioda na płytce wraca do migania, a panel nadal reaguje na przyciski.
- [ ] Po ponownym uruchomieniu serwera zmiany z panelu trafiają na stronę.
- [ ] Wyłączenie Wi-Fi na routerze: panel działa i sam wraca do sieci po włączeniu.
- [ ] Zmień stan na stronie przy odłączonym panelu: po połączeniu dioda się aktualizuje.
- [ ] Zmień ten sam klawisz najpierw na panelu offline, potem na stronie: po połączeniu wygrywa strona.
- [ ] Zmień ten sam klawisz najpierw na stronie, potem na panelu offline: po połączeniu wygrywa panel.

Serwer testowy trzyma dane w pamięci, więc jego restart przywraca stany OK.
Do testów konfliktów odłącz ESP32 od sieci, zamiast zatrzymywać serwer.

### Bez konfiguracji

- [ ] Z pustą nazwą Wi-Fi panel działa jak w `v0.1.0`, a log pokazuje brak konfiguracji.

Po zaliczeniu tej sekcji: tag `v0.2.0`.

## v0.3.0: pamięć po restarcie i konfiguracja

Przygotowanie jak dla `v0.2.0`. Stany zapisane przez starsze wersje firmware nie
są przenoszone, więc pierwszy start po aktualizacji pokazuje stany domyślne.

### Niewysłane zmiany

- [ ] Odłącz panel od sieci, zmień kolor przycisku i naciśnij EN: kolor wraca.
- [ ] Przywróć sieć: zmiana z panelu pojawia się na stronie.
- [ ] Zrób to samo, odłączając USB zamiast EN: zmiana też dociera na stronę.
- [ ] Odłącz panel od sieci, zmień kolor przycisku, zmień ten sam klawisz na
      stronie i naciśnij EN: po połączeniu wygrywa strona, a dioda się dostosowuje.

### Przypisania

- [ ] Odznacz „przypisany” na stronie: dioda gaśnie.
- [ ] Odłącz panel od sieci i naciśnij EN: ta dioda nadal jest zgaszona.

### Konfiguracja

- [ ] Zmiana jasności na stronie zmienia jasność diod.
- [ ] Zmiana koloru stanu na stronie zmienia kolor diod w tym stanie.
- [ ] Odłącz panel od sieci i naciśnij EN: jasność i kolory zostają.
- [ ] Zmień konfigurację przy odłączonym panelu: po połączeniu panel ją stosuje.

Po zaliczeniu tej sekcji: tag `v0.3.0`.
