# Diesel simulation

This fork adds a compression-ignition path while preserving the original
spark-ignition path.

The model is intended for real-time engine response and sound. It is not an
engineering combustion or emissions tool.

## Model structure

`CombustionEventController` is the common event source. `IgnitionModule`
implements the original spark events. `DieselInjectionModule` schedules
per-cylinder direct-injection events and performs fuel-cut above its configured
speed limit.

For a diesel engine:

1. The intake supplies air without premixed fuel.
2. Driver demand selects fuel mass between `idle_fuel_mass` and
   `max_fuel_mass`.
3. The injector adds that fuel directly to the cylinder over
   `injection_duration` crank angle.
4. Ignition delay changes with cylinder temperature, pressure, and cetane
   number.
5. A two-stage Wiebe heat-release curve represents the fast premixed burn and
   the slower diffusion burn.
6. Fuel cut replaces spark cut at the diesel speed limit.
7. Rapid cylinder pressure rise is mixed into the pressure-based exhaust audio
   path. `combustion_noise` controls its level.

Fuel scripts can now set `molecular_oxygen_ratio` and `product_mole_ratio`.
These values remove the original fixed gasoline reaction from the combustion
calculation. Existing fuel scripts keep the previous defaults.

## Script interface

Use `diesel_injection_module` instead of `ignition_module`. Existing
`ignition_wire` objects are also used as cylinder connection identifiers. They
do not represent spark plugs in a diesel script.

Important inputs are:

- `injection_timing_curve`: injection advance against engine speed.
- `idle_fuel_mass` and `max_fuel_mass`: fuel delivered per cylinder cycle.
- `injection_duration`: main-injection duration in crank angle.
- `ignition_delay`: reference delay at 850 K, 40 atm, and cetane 50.
- `cetane_number`: ignition-quality calibration input.
- `minimum_ignition_temperature`: temperature below which the delay timer
  pauses.
- `premixed_burn_fraction`: fuel fraction assigned to the fast burn.
- `premixed_burn_duration` and `diffusion_burn_duration`: staged heat-release
  durations in crank angle.
- `combustion_noise`: cylinder pressure-rise contribution to engine audio.

See `assets/engines/diesel/single_cylinder_diesel.mr` for the engine definition.
`assets/diesel-main.mr` is its executable script entry point.

## Validation

The automated tests cover:

- preservation of the default spark controller and gasoline chemistry;
- mole, composition, and temperature conservation during direct injection;
- fuel-specific diesel stoichiometry;
- per-cylinder injection event timing and driver-demand fuel quantity;
- fuel cut above the configured diesel speed limit;
- ignition-delay response to temperature, pressure, and cetane number;
- bounded, monotonic two-stage heat release;
- compilation and construction of the reference diesel script.

Before calibration changes are accepted, also check:

- existing gasoline scripts load and run without changed timing or output;
- no mixture fraction becomes negative or exceeds one;
- peak cylinder pressure occurs after top dead centre under normal load;
- injected fuel mass matches the fuel-consumption display;
- the engine starts at the configured cranking speed and does not self-ignite
  below the minimum ignition temperature;
- exhaust audio remains free of clipping across idle, load steps, and fuel cut.

## Current limits

The first model has one main injection event per cylinder cycle. The heat
release is calibrated rather than spray-resolved. Turbocharging, pilot and post
injection, EGR, smoke, emissions, glow plugs, and cold wall-film effects remain
future extensions.
