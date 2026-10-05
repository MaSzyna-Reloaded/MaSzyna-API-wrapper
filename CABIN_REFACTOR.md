# Kabina jak w oryginale: decyduje warstwa kabiny, VehicleServer trzyma wartość, Mover ją dostaje

## Kontekst
`90f2bd87` usunął wymuszone `CabOccupied=1` (36WE: każdy wagon z „zajętą” kabiną psuł przewód
główny) - zgodnie z oryginałem. Wyszło przy tym, że wrapper ma dwie „zajęte kabiny”: z
`driver_type` (`VehicleController::get_occupied_cab()` → `VehicleServer.vehicle_get_occupied_cab()`)
i żywą z Movera (`RailVehicleController::get_cabin_occupied()`, dump `cabin_occupied`, sygnał),
czyta się ją z Movera z powrotem, a ~25 miejsc sięga kontrolera po stan/konfig.

Oryginał (przeczytany):
* start: `headdriver`→1, `reardriver`→-1, inne→0 do `new TMoverParameters(..., Cab)`
  (DynObj.cpp:1940-1963);
* `CabOccupied` zapisują z zewnątrz: **TTrain** (gracz; `CabChange` Train.cpp:8516 - przy AI/cudzym
  członie samo `+= dir`, ręcznie `CabDeactivisationAuto`→`ChangeCab`→`InitializeCab`→
  `CabActivisationAuto`→`DirectionChange`; przejście przez pomost ±1 wg strony wejścia 8307/8339;
  `MoveToVehicle` zeruje w opuszczanym 10870/10928; polecenia `cabchangeforward/backward`
  485-486) i **Driver** (AI, Driver.cpp:2028-2086); Mover sam zmienia ją tylko w `ChangeCab`;
* `InitializeCab` (Train.cpp:8684): 1→`cab1definition:`, -1→`cab2definition:`, 0→`cab0definition:`
  z MMD, indeks w `TTrain::iCabn`;
* komenda kabiny może wymagać przycisku w tej kabinie: klakson (`OnCommand_hornlowactivate`,
  Train.cpp:7934-7942: bez `ggHornButton`/`ggHornLowButton` - log i nic).

TTrain to u nas warstwa kabiny (`CabinSystem` + `LegacyCabinLogic`, która działa na aktywnej
kabinie - jak oryginał); klawisze w `LegacyCabinLogic` i testy rozruchu zostają.

## 1. Kabina
* **Enum z danych** `VehicleServer.Cabin { CABIN_NONE, CABIN_0, CABIN_1, CABIN_2 }` - CABIN_N to
  `cabNdefinition:` MMD; kabin jest tyle, ile definicji. Mapowanie na `CabOccupied` (1→1, 2→-1,
  0/NONE→0) tylko w implementacji Movera (Train.cpp:8684).
* **VehicleServer trzyma wartość** (odpowiednik pola `CabOccupied`): pole w rekordzie `Vehicle`,
  start z `driver_type` (DynObj.cpp:1948-1963), `vehicle_set_cabin_occupied(rid, Cabin)` (zapis,
  przekazanie kontrolerowi, sygnał `vehicle_cabin_occupied_changed`), `vehicle_get_cabin_occupied`,
  klucz dumpa `cabin_occupied` dokłada serwer.
* **Mover tylko dostaje**: `MoverRailVehicleController` przepisuje wartość do `CabOccupied` (przy
  tworzeniu i przy zmianie); żadnej logiki aktywacji, nic nie czytane z powrotem.
* **Decyduje warstwa kabiny** (`CabinSystem`, odpowiednik TTrain): zmiana kabiny gracza (polecenia
  jak `cabchangeforward/backward`) robi sekwencję `CabChange` (dezaktywacja, zmiana, wybór
  definicji z MMD - tylko istniejącej, aktywacja, nawrotnik); AI zmienia kabinę przez tę samą
  warstwę (Driver.cpp:2028-2086).
* **Znika**: `VehicleController::get_occupied_cab()`, `vehicle_get_occupied_cab()`,
  `RailVehicleController::get_cabin_occupied()`/`prev_cabin_occupied`/`cabin_occupied_changed`,
  relay `RailVehicleServer.vehicle_occupied_cab_changed`, komenda pojazdu `cab_change`.
* `cab:int` w API `CabinSystem`/`CabinState`/`CabinLogic` → `VehicleServer.Cabin`, `cab_*` →
  `cabin_*`; czytelnicy (lusterka, low-poly, kamera zewnętrzna, player.gd, auto-rewident, bug
  report, widżety, python_screen_state, konsola, AI, Lua) przez serwer.
* **Przejęcie pojazdu, w którym nikt nie siedzi**: oryginał tego przypadku nie ma (gracz wchodzi
  tam, gdzie maszynista, albo pomostem) - która kabina, decyduje operator przed implementacją.

## 2. Bramka przycisku w kabinie (klakson)
* `MmdSemanticCatalog`: znacznik pozycji „komenda oryginału wymaga przycisku w tej kabinie” -
  tylko dla tych, których `OnCommand_*` sprawdza `SubModel == nullptr`, każdy potwierdzony w
  Train.cpp (klakson niski/wysoki/gwizdek na start).
* `LegacyCabinUnmodelledControls` nie podpina klawisza takiej kontrolki, gdy aktywna kabina jej
  nie ma; poprawić fałszywy komentarz „Every OnCommand_* works without its gauge”.
* Flagi `*_enabled` w `RailVehicleHorns` usuwa druga sesja (jej zmiana, niezacommitowana).

## 3. Reszta sięgania po kontroler
Typowane gettery serwera: `VehicleServer` - `vehicle_get_direction`, `vehicle_get_power`,
`vehicle_get_max_velocity`, `vehicle_get_mass_total` (+ istniejące `vehicle_get_dimensions`,
`vehicle_get_name`); `RailVehicleServer` - `vehicle_get_train_type`, `vehicle_get_coupler_stretched`,
`vehicle_get_train_damage`; load przez komponent. Przepinane: AI (`maszyna_legacy_driver_*`,
`ai_driver`, `auto_rewident`, `station`), dźwięk (`train_sound_system`, `brake_sound_model`,
`running_sound_model`), `external_camera.gd`, `RailVehicleRenderingServer.cpp:1006-1146`, helper
testów rozruchu. Zostaje składanie pojazdu. `CODE_STYLE.md`/`AGENTS.md`: konfig i stan tylko przez
serwer (dziś CODE_STYLE pozwala na `controller.max_velocity`).

## Kolejność (commity na słowo operatora)
1. CI `pipefail`, czyszczenie `test_zzz_ep07_main_switch_trip_diagnostic`, TODO → `--amend`.
2. Ta praca; SR61 (czerwony w pełnym przebiegu) zbadany przed nią. Przed edycją wspólnych plików
   (`mmd_semantic_catalog.gd`, `legacy/cabin/*`, instancery) `git diff` - druga sesja ma tam
   niezacommitowaną pracę; commit tylko po ścieżkach.

## Weryfikacja
build + `style-check` dotkniętych C++; parse-check `addons/libmaszyna` + `demo/hud`, probe-load
kabiny/HUD; grep: brak `get_occupied_cab`, `get_cabin_occupied`, `cab_change`,
`vehicle_get_controller(` poza składaniem; testy pojedynczo: `test_player_server`,
`test_zzz_vehicle_frame_rotation_regression`, `test_train_cab_change`, `test_zzz_ep07_cab_change`,
`test_rail_vehicle_rendering_server`, `test_legacy_cabin_*` (nowy: klakson w kabinie bez
`horn_bt` nie reaguje na klawisz, z przyciskiem reaguje), `test_driver_*`, dźwięk; partia
`scripts/run-startup-tests` i jeden proces GUT dla `test_zzz_startup` (jak CI).

## Dopisane 2026-10-05 (sweep data-driven - czeka na ten refaktor)
* `cabin_occupied` z domyślnym 1 (oryginał 0, MOVER.h:2091): `cabin_system.gd:135`,
  `developer_console.gd:85`, `python_screen_state.gd:169` → `vehicle_get_cabin_occupied()` z §1,
  bez domyślnej.
* Fallbacki i kopie domyślnych Movera w AI → typowane gettery serwera z §3, bez wymyślonych
  wartości: `driver_braking.gd:799, 975, 982`, `driver_pantographs.gd:46, 70, 78`,
  `driver_traction.gd:133-134, 324, 330, 380-393, 479`, `ai_driver.gd:695, 874`.
* `LegacyCabinDirectionKey` (C8, `legacy/cabin/direction_key.gd`) czyta `direction` z dumpa, aż §3
  da `vehicle_get_direction()`.
