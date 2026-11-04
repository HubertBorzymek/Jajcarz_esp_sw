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
Flash 4 MB, tablica partycji `partitions.csv` (ota_0, ota_1 po 1,875 MB).
Piny: `components/segments/src/board_rev0.h`.

## Wiążące decyzje

- Framework: ESP-IDF, nie PlatformIO/Arduino.
- ESP nie zna produktów, tylko `addr` (`segment.klawisz`). Segment = adres
  sprzętowy modułu, nie pozycja.
- Komunikacja: WebSocket + JSON po TLS, połączenie inicjuje ESP.
- `snapshot` przy każdym połączeniu jest główną synchronizacją. `state_set` to
  tylko przyspieszenie, bo panel bywa offline lub uśpiony.
- Klient aktualizuje się według `rev` z serwera, nie według `changed_by`.
- Konflikt: wygrywa nowsza zmiana, czas liczy serwer z `age_ms`.
- Po awarii wygrywa serwer: zmiana sprzed restartu idzie z `base_rev` i przegrywa
  z każdą zmianą wykonaną w tym czasie. `sync` nie podnosi wtedy `rev` klawisza.
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

Testy logiki bez płytki: `powershell -File tests\host\run.ps1` (MSVC z VS,
potrzebuje `managed_components`, czyli wcześniejszego `idf.py build`).
Testy na płytce: `docs/TESTY_MANUALNE.md`, dopisuj nowe przy każdej wersji.
Lokalne sekrety: `config/secrets.h`, tworzony ze wzoru przy pierwszym buildzie.
Pułapki:
- Pusty SSID sprawia, że kompilator usuwa kod sieciowy. Build bez Wi-Fi nie
  sprawdza linkowania `net`, do tego użyj tymczasowego SSID.
- Skopiowany plik zachowuje starą datę i ninja go pomija. Po `Copy-Item` na
  `secrets.h` ustaw `LastWriteTime`.
- Firmware z siecią ma około 1 MB, a slot 1,875 MB.
Konfiguracja projektu: `sdkconfig.defaults`, `sdkconfig` jest generowany.

Układ:

```text
main/main.c            start aplikacji
components/
  jajcarz_types/       typy wspólne: rgb_t, key_addr_t
  controller/          spina moduły, pętla główna, test startowy
  segments/            interfejs segmentu, sterownik GPIO + WS2812 (RMT)
  panel/               stany, cykl OK/LOW/OUT, debouncing, stan -> kolor
  presence/            tryby ACTIVE/DISPLAY/SLEEP
  status_led/          sygnalizator
  storage/             NVS: zapis i odczyt bloków danych
  net/                 Wi-Fi, WebSocket, kolejka odebranych wiadomości
  protocol/            kodowanie i dekodowanie JSON, bez sprzętu
  sync/                snapshot, ack, oczekujące zmiany, rev, bez sprzętu
```

`controller/src/link.c` spina `net`, `protocol` i `sync`. Klient WebSocket działa
w swoim zadaniu, a wiadomości są przetwarzane w pętli głównej.
`controller/src/persistence.c` raz na pętlę zapisuje różnice we flashu (stan,
przypisanie, oczekiwanie, `rev`, konfiguracja) i loguje zmiany stanów.
`storage` zna tylko bloki danych. Zmiana `key_record_t` lub `panel_config_t`
zmienia rozmiar bloku, więc stary zapis jest pomijany.

## Wersjonowanie

Praca na gałęzi `rev0`, wersje jako tagi `vX.Y.Z`. Numer firmware pochodzi
z `git describe`.

## Otwarte

- Tag `v0.1.0` po potwierdzeniu sprzętu przez właściciela: diody, klawisze,
  zapis stanów po restarcie, sygnalizator na GPIO2.
- Tagi `v0.2.0` i `v0.3.0` po testach z `docs/TESTY_MANUALNE.md`. Połączenie
  przez prawdziwe Wi-Fi nie było jeszcze sprawdzone.
- Prace wstrzymane do testów właściciela na sprzęcie. Nie zaczynaj nowego zakresu.
- Dalej: konfiguracja Wi-Fi bez przebudowy, OTA, czujnik, segmenty.
- `WS2812_MAX_PIXELS` to 5: dłuższy łańcuch wymaga większej pamięci RMT lub
  obsługi jej uzupełniania z przerwań.
