# CHANGELOG_AGENTS

Najnowsze na górze. Krótko: co, dlaczego, co dalej.

## 2026-11-04 (v0.3.0, bez taga)

- Dodano `controller/src/persistence.c`, `storage` przepisany na bloki danych.
- `sync_restore_key`: niewysłana zmiana po restarcie idzie bez `age_ms`.
- Protokół: obsługa `config` i `config` w `snapshot`. Błędna konfiguracja
  w `snapshot` nie unieważnia stanów klawiszy.
- Serwer: edycja konfiguracji na stronie, `POST /api/devices/{id}/config`.
  Naprawiono zatrzymywanie odświeżania strony po kliknięciu pola wyboru.
- Stare zapisy `key<seg>.<key>` są pomijane, nazwy to teraz `k<seg>.<key>`.
- Zweryfikowano: testy hosta OK, 18 testów serwera OK, build i start na płytce
  z tymczasowym SSID i bez konfiguracji, także po restarcie.
- Niezweryfikowane: zapis i odczyt po naciśnięciu przycisków, prawdziwe Wi-Fi.
- Na płytce: firmware bez konfiguracji z kodem tej wersji.
- Następny krok: testy właściciela, tagi, potem decyzja o kolejnym zakresie.

## 2026-11-04 (v0.2.0, bez taga)

- Dodano `net`, `protocol`, `sync`, `controller/src/link.c`, `config/secrets.example.h`.
- Zależności: `esp_websocket_client` 1.8.0, `cjson` 1.7.19, wersje w `dependencies.lock`.
- Serwer testowy gotowy, repo `Jajcarz_test_server`. Znaleziono i poprawiono brak
  odpowiedzi serwera na zamknięcie WebSocket.
- Zweryfikowano: testy hosta OK, 15 testów serwera OK, serwer na Kestrelu (strona,
  API, `hello` -> `snapshot`), build z tymczasowym SSID linkuje `net` i startuje na
  płytce, ponawia Wi-Fi bez awarii. Wersja bez konfiguracji działa offline.
- Niezweryfikowane: prawdziwe Wi-Fi i ESP32 z serwerem, całość testów manualnych.
- Na płytce: firmware bez konfiguracji, `v0.0.0-2-g08d49b2-dirty`.
- Następny krok: testy właściciela, tagi `v0.1.0` i `v0.2.0`, potem `v0.3.0`.

## 2026-11-04 (v0.1.0, bez taga)

- `main.c` podzielony na komponenty, układ w `README_AGENTS.md`.
- Dodano: `key_addr_t`, sterownik segmentu, `presence`, `status_led`, `storage`,
  `partitions.csv`, `sdkconfig.defaults`, `tests/host`.
- RMT: 128 symboli zamiast 64, żeby 4 diody nie wymagały uzupełniania z przerwań.
  Ważne przy Wi-Fi i zapisie do flasha.
- Zweryfikowano: czysty build bez ostrzeżeń, testy hosta OK, wgranie z hashem OK,
  start bez błędów, 4 klawisze w logu.
- Niezweryfikowane: fizyczne działanie diod, klawiszy, zapisu i GPIO2.
- Następny krok: potwierdzenie właściciela, tag `v0.1.0`, potem `v0.2.0`.

## 2026-11-04 (dokumentacja)

- Zamknięto założenia i zapisano je w `README_AGENTS.md`, protokół v1 w `docs/PROTOCOL.md`.
- Dodano do protokołu ostatnią zmianę na serwerze: `rev`, `changed_by`, `changed_at`.
- Założono repo serwera testowego `Jajcarz_test_server` (bez kodu).
- Kod firmware bez zmian.
- Następny krok: `v0.1.0`, tylko po zgodzie właściciela.

## 2026-11-04 (v0.0.0)

- Repo git, gałąź `rev0`, baseline `98d722c`, tag `v0.0.0`.
- Build OK. Wgranie nietestowane. Działanie kodu na sprzęcie niepotwierdzone.
