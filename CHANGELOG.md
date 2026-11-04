# Historia zmian

## Niewydane

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
