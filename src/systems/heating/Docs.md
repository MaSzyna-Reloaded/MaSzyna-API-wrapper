# Heating system notes

This module represents `Clima:` section from `.fiz` files.

Current original MaSzyna implementation assumes 2 sources of heating
(main and alternative). We could future-proof it by allowing multiple
heating sources (array), but for now we will stick to the original design.

Given section may look like this:

```fizfile
Clima: Heating=PowerCable HPowerTrans=ElectricPower HMaxVoltage=3000 AlterHeating=PowerCable AlterHPowerTrans=SteamPower AlterHSteamPress=0.45
```

`LoadFIZ_Clima` loads 2 fields in `Mover.cpp`: `HeatingPowerSource` and `AlterHeatPowerSource`.

First `TPowerParameters::SourceType` is set (just an enum values based on string), then
`LoadFIZ_PowerParamsDecode` is used to load rest of the parameters.
