# Testy manualne

Testy wymagające płytki. Wynik zaznacz `[x]`, a problem opisz pod punktem.

## v0.1.0: panel bez sieci

Przygotowanie: płytka podłączona przez USB, w PowerShell:

```powershell
. C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1
idf.py -p COM3 flash monitor
```

Wyjście z monitora: `Ctrl+]`. Pozostaw monitor otwarty, żeby widzieć log.

### Start

- [ ] Po resecie (EN) wszystkie 4 diody świecą kolejno na czerwono, zielono, niebiesko.
- [ ] Każdy kolor trwa około 0,3 s.
- [ ] Log pokazuje `Jajcarz <wersja>` i `Gotowe, klawisze: 4`.
- [ ] Przy pierwszym uruchomieniu po teście wszystkie diody są zielone.

### Przyciski

| Przycisk | Adres w logu |
|---|---|
| SW1 | 0.0 |
| SW2 | 0.1 |
| SW3 | 0.2 |
| SW4 | 0.3 |

- [ ] SW1: zielony → żółty → czerwony → zielony, zmienia się tylko dioda 1.
- [ ] SW2: to samo, tylko dioda 2.
- [ ] SW3: to samo, tylko dioda 3.
- [ ] SW4: to samo, tylko dioda 4.
- [ ] Każde naciśnięcie daje dokładnie jeden wpis w logu, np. `0.1 -> LOW`.
- [ ] Przytrzymanie przez 3 s daje jedną zmianę.
- [ ] Szybkie klikanie nie daje podwójnych zmian.
- [ ] Przytrzymanie przycisku podczas resetu nie zmienia stanu.
- [ ] Żółty nie wpada w zieleń.

### Pamięć stanów

- [ ] Ustaw różne kolory i naciśnij EN: po teście startowym kolory wracają.
- [ ] Ustaw różne kolory i odłącz USB na 5 s: po podłączeniu kolory wracają.

### Dioda sygnalizacyjna

- [ ] Dioda na płytce miga mniej więcej raz na sekundę.

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
