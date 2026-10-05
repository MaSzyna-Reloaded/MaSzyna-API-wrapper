# Kabina jak w oryginale - co zostało po obsadzie

## Zrobione (obsada)
Dawny §1 („numer zajętej kabiny”) zastąpiła obsada: `PersonServer` (osoby - gracz, każdy AI),
kabiny jako RIDy `VehicleServer` (`cabin_create`, `vehicle_cabin_attach`), osoby w kabinach w rolach
`VehiclePersonRole` (DRIVER, OBSERVER; jeden DRIVER na kabinę), `RailVehicleServer` tłumaczy na
rodzaje kabin (`RailVehicleCabinKind` FRONT/MACHINE/REAR, skróty `person_enter_front_cabin` itd.)
i podaje kontrolerowi rodzaj kabiny DRIVERa (`CabOccupied` tylko w `MoverRailVehicleController`).
Sterowanie to aktywacja kabiny, nie ma roli składu. `DriverSystem` działa na osobach, `PlayerServer`
ma swoją osobę (F5 - DRIVER i pełne wyjście z poprzedniego pojazdu, F4 - tylko widok, przełącznik
AI/Gracz tylko na chipie pojazdu gracza). Warstwa kabiny (`CabinSystem`, zachowania, widżety) jest
kluczowana RIDem kabiny; zmiana kabiny - `CabinSystem.person_change_cabin()` (TTrain::CabChange).

## 2. Bramka przycisku w kabinie (klakson)
* `MmdSemanticCatalog`: znacznik pozycji „komenda oryginału wymaga przycisku w tej kabinie” -
  tylko dla tych, których `OnCommand_*` sprawdza `SubModel == nullptr`, każdy potwierdzony w
  Train.cpp (klakson niski/wysoki/gwizdek na start).
* `LegacyCabinUnmodelledControls` nie podpina klawisza takiej kontrolki, gdy kabina kierującego jej
  nie ma; poprawić fałszywy komentarz „Every OnCommand_* works without its gauge”.

## 3. Reszta sięgania po kontroler
Typowane gettery serwera: `VehicleServer` - `vehicle_get_direction`, `vehicle_get_power`,
`vehicle_get_max_velocity`, `vehicle_get_mass_total` (+ istniejące `vehicle_get_dimensions`,
`vehicle_get_name`); `RailVehicleServer` - `vehicle_get_train_type`, `vehicle_get_coupler_stretched`,
`vehicle_get_train_damage`; load przez komponent. Przepinane: AI (`maszyna_legacy_driver_*`,
`ai_driver`, `auto_rewident`, `station`), dźwięk (`train_sound_system`, `brake_sound_model`,
`running_sound_model`), `external_camera.gd`, `RailVehicleRenderingServer.cpp` (dym, ładunek), helper
testów rozruchu. Zostaje składanie pojazdu. `CODE_STYLE.md`/`AGENTS.md`: konfig i stan tylko przez
serwer (dziś CODE_STYLE pozwala na `controller.max_velocity`).
* Fallbacki i kopie domyślnych Movera w AI → typowane gettery serwera, bez wymyślonych wartości:
  `driver_braking.gd:799, 975, 982`, `driver_pantographs.gd:46, 70, 78`,
  `driver_traction.gd:133-134, 324, 330, 380-393, 479`, `ai_driver.gd:695, 874`.
* `LegacyCabinDirectionKey` (`legacy/cabin/direction_key.gd`) czyta `direction` z dumpa, aż będzie
  `vehicle_get_direction()`.

## Weryfikacja
build + `style-check` dotkniętych C++; parse-check dotkniętych `.gd`; grep: `vehicle_get_controller(`
poza składaniem; testy pojedynczo: `test_legacy_cabin_*` (nowy: klakson w kabinie bez `horn_bt`
nie reaguje na klawisz, z przyciskiem reaguje), `test_driver_*`, dźwięk; partia
`scripts/run-startup-tests` i jeden proces GUT dla `test_zzz_startup` (jak CI).
