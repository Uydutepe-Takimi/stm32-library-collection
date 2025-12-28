# Changelog

## Next Release

+ **[ENHANCEMENT]** I2C: Add I2c class and related concepts for managing I2C functionality.

+ **[ENHANCEMENT]** Spi: Add Spi class and related concepts for managing SPI functionality.

+ **[ENHANCEMENT]** Utility: Introduce __IsMessage concept and __ClampMessageLength utility for message buffer validation and size clamping.

+ **[ENHANCEMENT]** Utility: Make __CallbackManager a self-registering RAII callback manager.

+ **[ENHANCEMENT]** Utility: Rename __FixedCallback to __InplaceFunction.

+ **[ENHANCEMENT]** Utility: Introduce __FixedCallback for fixed-size callable wrappers replacing std::function.

+ **[DOCUMENTATION]** Docs: Refactor documentation and improve clarity.

+ **[ENHANCEMENT]** Uart: Enhance Uart class with supporting different working modes, callbacks, and improved documentation.

+ **[ENHANCEMENT]** Utility: Implement callback manager for STM32 HAL peripherals.

+ **[ENHANCEMENT]** Utility: Add STM32_UNIQUE_TAG for unique tag type generation.

+ **[ENHANCEMENT]** Utility: Reorganize Utility functions into separate internal headers.

+ **[ENHANCEMENT]** Adc: Enhance Adc class with more readable template parameters and improved documentation.

+ **[ENHANCEMENT]** Dac: Enhance Dac class with more readable template parameters and better documentation.

+ **[ENHANCEMENT]** Utility: Add range_size to Range struct and update IsRange concept.

+ **[ENHANCEMENT]** Timer: Enhance Timer class with improving documentation.

+ **[ENHANCEMENT]** Servo: Separate Servo definition from Pwm.hpp to Servo.hpp.

+ **[ENHANCEMENT]** Pwm: Enhance Pwm class with more readable template parameters and better documentation.

+ **[ENHANCEMENT]** Libs: Add Utility.hpp with Constant and Range utility structs and concepts.

+ **[ENHANCEMENT]** Libs: Add Config.hpp for working mode configurations.

+ **[DOCUMENTATION]** Docs: Update README with more detailed instructions.

+ **[ENHANCEMENT]** Gpio: Refactor GPIO class.

+ **[DOCUMENTATION]** Docs: Add issue templates.

+ **[ENHANCEMENT]** Docs: Add pull request template.

+ **[ENHANCEMENT]** Libs: Change namespace name to STM32.

## [v1.0.0] - 08-12-2025

+ Initial release.
