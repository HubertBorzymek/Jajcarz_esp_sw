# README_AGENTS

Skrót dla agentów. Historia: `CHANGELOG_AGENTS.md`. Protokół: `docs/PROTOCOL.md`.
Przy każdej zmianie dopisz wpis do obu changelogów.

## Projekt

Firmware panelu na lodówkę: klawisz + dioda = stan produktu (OK/LOW/OUT).
Serwer na Azure jest źródłem prawdy, aplikacja webowa to osobny projekt.
Zakres tego repo: tylko ESP32.

## Środowisko

- ESP-IDF 6.1 (EIM), C, układ `esp32`, port COM3.
- `export.ps1` nie działa. Aktywacja w każdym wywołaniu PowerShell:
  `. C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1 *> $null`
- Esptool: `& "$env:IDF_PYTHON_ENV_PATH\Scripts\python.exe" -m esptool --port COM3 ...`
- Monitor portu w VS Code blokuje COM3.
- Serwer testowy: `C:\Users\leemi\Documents\Projects\Visual Studio\Jajcarz_test_server`
  (osobne repo, ASP.NET Core, net10.0, VS 2026).

## Sprzęt rev0

LED WS2812B-V6 ×4: GPIO13, GRB, 800 kHz. Klawisze do GND z pull-upem:
GPIO32, 33, 25, 26 (`addr` 0.0 do 0.3). Sygnalizator: GPIO2, do potwierdzenia.
Flash fizycznie 4 MB, w konfiguracji jeszcze 2 MB.

## Wiążące decyzje

- Framework: ESP-IDF, nie PlatformIO/Arduino.
- ESP nie zna produktów, tylko `addr` (`segment.klawisz`). Segment = adres
  sprzętowy modułu, nie pozycja.
- Komunikacja: WebSocket + JSON po TLS, połączenie inicjuje ESP.
- `snapshot` przy każdym połączeniu jest główną synchronizacją. `state_set` to
  tylko przyspieszenie, bo panel bywa offline lub uśpiony.
- Klient aktualizuje się według `rev` z serwera, nie według `changed_by`.
- Konflikt: wygrywa nowsza zmiana, czas liczy serwer z `age_ms`.
- Offline: dla każdego klawisza kolejka trzyma tylko ostatnią zmianę, we flashu.
- Diody produktów świecą stale. Stan urządzenia tylko na sygnalizatorze.
  Jedyny wyjątek: `identify`.
- Klawisz nieprzypisany: dioda zgaszona, bez rozbłysku, wysyła `unassigned_press`.
- Do pierwszego połączenia z serwerem (w tym całe `v0.1.0`) klawisze traktuj jak
  przypisane, inaczej panel byłby ciemny.
- Tryby `ACTIVE`/`DISPLAY`/`SLEEP` istnieją od `v0.1.0`. Bez czujnika zawsze `ACTIVE`.
- Partycje OTA od `v0.1.0`, sama funkcja OTA później.
- Dane Wi-Fi i tokeny nigdy w repo.

## Standard kodu

Priorytet właściciela projektu: czysty, czytelny kod podzielony na pliki i foldery.

- Jeden moduł, jedna odpowiedzialność. Cel: plik do ok. 300 linii.
- Logika w `components/<nazwa>/` z `include/` (publiczne API) i `src/`.
- Zależności jawne przez `REQUIRES`. Bez globali współdzielonych między modułami.
- `main/main.c` tylko inicjalizuje i uruchamia moduły.
- Logika bez sprzętu i bez FreeRTOS, żeby dało się ją testować na hoście.
- Moduły wyższego poziomu nie znają pinów ani sposobu podłączenia segmentu.
- Nazwy w kodzie po angielsku, komentarze po polsku, z prefiksem modułu w API.

Docelowy układ:

```text
main/main.c            start aplikacji
components/
  segments/            interfejs segmentu, sterownik GPIO + WS2812 (RMT)
  panel/               stany, cykl OK/LOW/OUT, adresy, stan -> kolor
  presence/            tryby ACTIVE/DISPLAY/SLEEP
  status_led/          sygnalizator
  storage/             NVS: stany, konfiguracja, kolejka zmian
  net/                 Wi-Fi, WebSocket (v0.2.0)
  protocol/            kodowanie i dekodowanie JSON (v0.2.0)
  sync/                snapshot, ack, kolejka, konflikty (v0.2.0-v0.3.0)
```

## Wersjonowanie

Praca na gałęzi `rev0`, wersje jako tagi `vX.Y.Z`. Numer firmware pochodzi
z `git describe`.

## Do zrobienia przed v0.1.0

- `sdkconfig` jest w `.gitignore`: dodać `sdkconfig.defaults` bez sekretów.
- Ustawić flash na 4 MB i tablicę partycji z OTA.
- Potwierdzić diodę na GPIO2.
- Sprawdzić komponent klienta WebSocket dla IDF 6.1 (przed `v0.2.0`).
