# Sabertrio Lightsaber - ProffieOS Configuration

This repository is a fork of the mainline ProffieOS repository, with my own modifications
layered on to match the blade styles that were shipped out of box from Sabertrio.

I've tested this repository on my Sabertrio Vektra saber that I got as part of Sabertrio's
2021 Black Friday sale; it's the ready-to-ship (RTS) one with the purple activation switch.

My motivations are:

* Make the saber more responsive. Out of box, the activation switch didn't seem to always
  trigger correctly. Powering it on would sometimes be frustrating, and features like the
  "color wheel" (aka Kyber Dial) would be very difficult to engage.
* Be able to use the mainline proffie configurations, such as Fett263's button configuration
  for two-button sabers.
* Clean up the code in `s3config.h`:
  - General code cleanup. The indentation was all over the place and the blade styles are
    very hard to read out-of-box.
  - Simplify the `blades[]` array. Sabertrio was smart to build a configuration file in a
    way that they can use the same file for all of their sabers, but it adds in a lot of
    extra overhead for sabers that don't need it. A large amount of it comes from the extra
    blades that are present on the Krosgaard saber.

# Key Files

I suspect that outside of these files, the rest of ProffieOS is unchanged. As such this is where I'm focusing most of my attention
on cleanup and tweaking.

* [config/default_proffieboard_config.h](config/default_proffieboard_config.h) (previously `config/s3config.h`)

# References and Notes

## Example blade configuration (Sabertiro default)

Tip: Don't like the color of the activation switch your saber shipped with? Set the 4th "blade" in the
below config to `StylePtr<Black>()` and it'll turn off the LED.

```c

Preset blade[] = {
		{
		// Soundfont Directory Name
		"0-NO_BLADE",

		// Leave blank - unused?
		"",

		// Primary Blade Style
		StylePtr<Black>(),

		// Cross-guard blade styles. Probably ignored unless you have a Krosgaard saber.
		StylePtr<Black>(),
		StylePtr<Black>(),

		// Activation Switch
		StylePtr<WHITE>(),

		// Blade preset profile name - not sure how this is used, but should probably be unique.
		"ProfileName"
	}
}

```


## Disable Main Button LED

##  Blade Settings

* 1-SABERTRIO Fully Responsive RandomFlicker Blade Style ( DodgerBlue ) (animated)
* 2-DARK_EDITION Fully Responsive RandomFlicker Blade Style ( Red ) (animated)
* 3-THE_LIGHT Fully Responsive RandomFlicker Blade Style ( DeepSkyBlue ) (original)
* 4-THE_BALANCE Fully Responsive RandomFlicker Blade Style ( Green ) (prequel)
* 5-THE_DARK Fully Responsive RandomFlicker Blade Style ( Red ) (original)
* 6-VENGEANCE Fully Responsive RandomFlicker Blade Style ( Red ) (animated)
* 7-KROSSGUARD Fully Responsive Kylo Ren Unstable Neopixel Lightsaber (Unstable Swing) Styles ( Red ) (sequel)
* 8-SNIPPETS Fully Responsive AudioFlicker Canon Jedi/Sith Neopixel Lightsaber Styles ( Silver ) (animated)
* 9-MENACE Fully Responsive RandomFlicker Blade Style ( Red ) (prequel)
* 10-TEMPLE_GUARDIAN Fully Responsive RandomFlicker Blade Style ( Gold ) (animated)
* 11-SHOCK_BATON Shock Baton Neopixel Lightsaber Style ( Shock Baton ) (sequel) (polar spark,center in)
* 12-THE_TRAGEDY Fully Responsive AudioFlicker Canon Jedi/Sith Neopixel Lightsaber Styles ( Red ) (prequel)
* 13-TEENY_SF Fully Responsive RandomFlicker Blade Style ( Cyan ) (prequel)
* 14-SMOOTH_JEDI Fully Responsive RandomFlicker Blade Style ( Blue ) (original)
* 15-SMOOTH_GREY Fully Responsive AudioFlicker Canon Jedi/Sith Neopixel Lightsaber Styles ( Amber/Gold ) (sequel)
* 16-SMOOTH_FUZZ Fully Responsive RandomFlicker Blade Style ( Purple ) (prequel)
* 17-ROGUE_COMMANDER Fully Responsive "Power Surge" Unstable Neopixel Lightsaber Style ( Red ) (original)
* 18-TYTHONIAN_CYRSTAL Fully Responsive RandomFlicker Blade Style ( Green ) (original)
