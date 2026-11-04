# CHANGELOG_AGENTS

Najnowsze na górze. Krótko: co, dlaczego, co dalej.

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
