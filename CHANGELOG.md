# Historia zmian

## v0.3.0 - 2026-11-04

### Dodano
- Niewysłane zmiany, przypisania i numery zmian przetrwają restart.
- Kolory i jasność ustawiane z serwera i zapisywane we flashu.
- Po awarii zasilania źródłem prawdy jest serwer: zmiana sprzed restartu przegrywa
  ze zmianą wykonaną w tym czasie.

## v0.2.0 - 2026-11-04

### Dodano
- Lista testów manualnych: `docs/TESTY_MANUALNE.md`.
- Połączenie z serwerem: Wi-Fi, WebSocket i protokół v1.
- Synchronizacja stanów w obie strony, praca bez sieci do restartu, konflikty.
- Przypisywanie klawiszy i identyfikacja klawisza miganiem.
- Lokalne dane w `config/secrets.h`, poza gitem.

## v0.1.0 - 2026-11-04

### Dodano
- Założenia projektu i dokumentacja: `README.md`, `docs/PROTOCOL.md`.
- Dokumentacja dla agentów: `README_AGENTS.md`, `CHANGELOG_AGENTS.md`, `CLAUDE.md`.
- Adresy klawiszy `segment.klawisz` i obsługa segmentów przez wymienny sterownik.
- Zapis stanów klawiszy we flashu: panel pamięta je po restarcie.
- Tryby obecności (na razie bez czujnika, zawsze pełna obsługa).
- Sygnalizator na diodzie płytki: bez serwera wolno miga.
- Partycje pod aktualizacje OTA i konfiguracja w `sdkconfig.defaults`.
- Numer wersji firmware w logu startowym.
- Testy logiki uruchamiane bez płytki.

### Zmieniono
- Kod podzielony z jednego pliku na komponenty.
- Log zmiany stanu używa adresu klawisza, np. `0.1 -> LOW` zamiast `SW2 -> LOW`.
- Flash ustawiony na faktyczne 4 MB.

## v0.0.0 - 2026-11-04

### Dodano
- Firmware testowy: 4 przyciski i 4 diody WS2812, cykl OK/LOW/OUT, bez sieci.
