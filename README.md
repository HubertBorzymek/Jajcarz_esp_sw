# Jajcarz

Panel na lodówkę do oznaczania stanu produktów. Przy każdym produkcie jest
przycisk i dioda: **jest**, **kończy się** albo **nie ma**.

Status można zmienić od razu przy lodówce, bez telefonu. W sklepie sprawdzisz
go w aplikacji webowej na telefonie lub komputerze i tam zmienisz po zakupach.

To repozytorium zawiera **tylko firmware ESP32**.

## Części systemu

| Część | Gdzie | Rola |
|---|---|---|
| Firmware panelu | to repozytorium | przyciski, diody, synchronizacja |
| Aplikacja webowa | osobny projekt (Azure) | produkty, przypisania, statusy, konfiguracja |
| Serwer testowy | `Projects\Visual Studio\Jajcarz_test_server` | testowanie firmware bez aplikacji |

Komunikację opisuje [docs/PROTOCOL.md](docs/PROTOCOL.md).

## Jak to działa

- Serwer jest źródłem prawdy i przechowuje ostatnią zmianę każdego klawisza.
- Klawisze mają tylko adresy, a produkty przypisuje do nich aplikacja webowa.
- Panel działa też bez sieci. Zmiany czekają we flashu i są wysyłane po
  połączeniu, a przy konflikcie wygrywa nowsza zmiana.

## Panel

| Stan | Kolor | Znaczenie |
|---|---|---|
| OK | zielony | jest |
| LOW | żółty | kończy się |
| OUT | czerwony | nie ma |

Naciśnięcie zmienia stan: OK → LOW → OUT → OK. Diody produktów świecą stale
i pokazują tylko stan produktu. Klawisz bez produktu ma zgaszoną diodę.

Stan urządzenia pokazuje osobny sygnalizator:

| Sygnalizator | Znaczenie |
|---|---|
| świeci stale | połączono z serwerem |
| wolno miga | brak połączenia |
| krótko mignie | zmiana potwierdzona przez serwer |

## Sprzęt: prototyp rev0

ESP32 DevKit V1 z ESP32-WROOM-32 i 4 MB flash, na uniwersalce.

| Element | Pin |
|---|---|
| dane WS2812B-V6 (4 szt., przez bufor 5 V) | GPIO13 |
| SW1 / SW2 / SW3 / SW4 (do GND) | GPIO32 / 33 / 25 / 26 |
| sygnalizator: dioda na płytce | GPIO2 |

Docelowo płyta główna i moduły z przyciskami (1+n segmentów). Później dojdzie
czujnik odległości z trzema trybami: uśpienie, same diody, pełna obsługa.

## Budowanie

Wymagany ESP-IDF 6.1. Z katalogu projektu, w PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
. C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1
idf.py build
idf.py -p COM3 flash monitor
```

Pierwszy build tworzy `config/secrets.h` (plik poza gitem). Wpisz tam dane Wi-Fi,
adres serwera i token. Puste Wi-Fi oznacza pracę bez sieci.

Testy manualne na płytce: [docs/TESTY_MANUALNE.md](docs/TESTY_MANUALNE.md).

Testy logiki bez płytki (wymagają Visual Studio z narzędziami C++):

```powershell
powershell -ExecutionPolicy Bypass -File tests\host\run.ps1
```

## Wersje

- Gałąź `revN` oznacza generację prototypu, obecnie `rev0`.
- Tag `vX.Y.Z` oznacza wersję oprogramowania.

| Wersja | Zakres |
|---|---|
| `v0.0.0` | firmware testowy bez sieci |
| `v0.1.0` | podział na moduły, adresy, tryby, sygnalizator, zapis stanów, partycje OTA |
| `v0.2.0` | Wi-Fi, WebSocket, protokół, praca offline, konflikty |
| `v0.3.0` | niewysłane zmiany, przypisania i konfiguracja przetrwają restart |
| później | konfiguracja Wi-Fi bez przebudowy, OTA, czujnik, segmenty |

Historia zmian: [CHANGELOG.md](CHANGELOG.md).
