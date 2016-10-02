module Flipper.Console.Options where

import Control.Applicative

import Data.Word

import Flipper
import Flipper.GPIO (DigitalPin(..), AnalogPin(..), Direction(..))
import Flipper.LED

import Flipper.Console.Action
import Flipper.Console.Parsers

import Options.Applicative hiding (action)

import qualified Text.Megaparsec        as M
import qualified Text.Megaparsec.String as M

data Options = Options {
    optEndpoint :: Endpoint
  , optOneoff   :: Maybe ConsoleAction
  } deriving (Eq, Ord, Show)

options :: Parser Options
options = Options <$> endpoint <*> (optional action)

endpoint :: Parser Endpoint
endpoint = option (readParser parseEndpoint) opt
    where opt = mconcat [ long "endpoint"
                        , short 'e'
                        , metavar "ENDPOINT"
                        , help "The endpoint via which Flipper will be attached."
                        , value (USB Nothing)
                        ]

readParser :: M.Parser a -> ReadM a
readParser p = eitherReader (showLeft . M.runParser p "")
    where showLeft (Right x) = (Right x)
          showLeft (Left e)  = (Left (show e))

action :: Parser ConsoleAction
action = hsubparser (mconcat builtins <> mconcat calls)
    where builtins = [ command "flash" flash
                     , command "install" install
                     , command "launch" launch
                     , command "reset" reset
                     , command "suspend" suspend
                     , command "format" format
                     ]
          calls = [ command "button" ((ConsoleCall . ButtonCall) <$> button)
                  , command "fs" ((ConsoleCall . FSCall) <$> fs)
                  , command "gpio" ((ConsoleCall . GPIOCall) <$> gpio)
                  , command "led" ((ConsoleCall . LEDCall) <$> led)
                  , command "spi" ((ConsoleCall . SPICall) <$> spi)
                  , command "uart" ((ConsoleCall . UARTCall) <$> uart)
                  ]

flash :: ParserInfo ConsoleAction
flash = info (Flash <$> flashP) flashI
    where flashI = mconcat [ fullDesc
                           , progDesc "Flash firmware to the device."
                           ]
          flashP = strArgument $ mconcat [ metavar "FIRMWARE"
                                         , help "Firmware file image."
                                         ]

install :: ParserInfo ConsoleAction
install = info (Install <$> moduleID <*> installP) installI
    where installI = mconcat [ fullDesc
                             , progDesc "Install a module on the device."
                             ]
          installP = strArgument $ mconcat [ metavar "MODULE_FILE"
                                           , help "Module file image."
                                           ]

launch :: ParserInfo ConsoleAction
launch = info (Launch <$> launchP) launchI
    where launchI = mconcat [ fullDesc
                            , progDesc "Launch a module on the device."
                            ]
          launchP = strArgument $ mconcat [ metavar "MODULE"
                                          , help "Module name."
                                          ]

reset :: ParserInfo ConsoleAction
reset = info (pure Reset) resetI
    where resetI = mconcat [ fullDesc
                           , progDesc "Reset the device."
                           ]

suspend :: ParserInfo ConsoleAction
suspend = info (pure Suspend) suspendI
    where suspendI = mconcat [ fullDesc
                             , progDesc "Suspend the device."
                             ]

format :: ParserInfo ConsoleAction
format = info (pure Format) formatI
    where formatI = mconcat [ fullDesc
                            , progDesc "Format the flash memory on the device."
                            ]

button :: ParserInfo ButtonAction
button = info (hsubparser buttonRead) buttonI
    where buttonI = mconcat [ fullDesc
                            , progDesc "Read the device's button state."
                            ]

fs :: ParserInfo FSAction
fs = info fsP fsI
    where fsP = hsubparser $ mconcat [ fsCreateFromString
--                                   , fsCreateFromFile
                                     , fsRemove
                                     , fsRename
                                     ]
          fsI = mconcat [ fullDesc
                        , progDesc "Interact with the device's file system."
                        ]

gpio :: ParserInfo GPIOAction
gpio = info gpioP gpioI
    where gpioP = hsubparser $ mconcat [ gpioDirection
                                       , gpioRead
                                       , gpioWrite
                                       ]
          gpioI = mconcat  [ fullDesc
                           , progDesc "Interact with the device's GPIO pins."
                           ]

led :: ParserInfo LEDAction
led = info (hsubparser ledRGB) ledI
    where ledI = mconcat [ fullDesc
                         , progDesc "Set the device's RGB LED state."
                         ]

spi :: ParserInfo SPIAction
spi = info spiP spiI
    where spiI = mconcat [ fullDesc
                         , progDesc "Interact with the device's SPI bus."
                         ]
          spiP = hsubparser $ mconcat [ spiEnable
                                      , spiDisable
                                      , spiRead
                                      , spiWriteFromString
--                                    , spiWriteFromFile
                                      ]

uart :: ParserInfo UARTAction
uart = info uartP uartI
    where uartI = mconcat [ fullDesc
                          , progDesc "Interact with the device's UART bus."
                          ]
          uartP = hsubparser $ mconcat [ uartEnable
                                       , uartDisable
                                       , uartRead
                                       , uartWriteFromString
--                                     , uartWriteFromFile
                                       ]

buttonRead :: Mod CommandFields ButtonAction
buttonRead = command "read" (info (pure ButtonRead) readI)
    where readI = mconcat [ fullDesc
                          , progDesc "Read the device's button state."
                          ]

fsCreateFromString :: Mod CommandFields FSAction
fsCreateFromString = command "create" (info (FSCreateFromString <$> strArgument fnameP <*> strArgument stringP) createI)
    where fnameP = mconcat [ help "File name to create."
                           , metavar "FILENAME"
                           ]
          stringP = mconcat [ help "File contents as quoted string."
                            , metavar "CONTENTS"
                            ]
          createI = mconcat [ fullDesc
                            , progDesc "Create a file on the device from a user-provided string."
                            ]

fsRemove :: Mod CommandFields FSAction
fsRemove = command "remove" (info (FSRemove <$> strArgument fnameP) removeI)
    where fnameP = mconcat [ help "File name to remove."
                           , metavar "FILENAME"
                           ]
          removeI = mconcat [ fullDesc
                            , progDesc "Remove a file on the device."
                            ]

fsRename :: Mod CommandFields FSAction
fsRename = command "rename" (info (FSRename <$> strArgument fromP <*> strArgument toP) renameI)
    where fromP = mconcat [ help "Old file name."
                          , metavar "FROM"
                          ]
          toP = mconcat [ help "New file name."
                        , metavar "TO"
                        ]
          renameI = mconcat [ fullDesc
                            , progDesc "Rename a file on the device."
                            ]

gpioDirection :: Mod CommandFields GPIOAction
gpioDirection = command "direction" (info (digitalDirection <|> analogDirection) directionI)
    where directionI = mconcat [ fullDesc
                               , progDesc "Set a GPIO pin's I/O direction."
                               ]

digitalPin :: Parser DigitalPin
digitalPin = argument (readParser parseDigitalPin) dpinP
    where dpinP = mconcat [ metavar "DIGITAL_PIN"
                          , help "Digital GPIO pin."
                          ]

analogPin :: Parser AnalogPin
analogPin = argument (readParser parseAnalogPin) apinP
    where apinP = mconcat [ metavar "ANALOG_PIN"
                          , help "Analog GPIO pin."
                          ]

direction :: Parser Direction
direction = argument (readParser parseDirection) dirP
    where dirP = mconcat [ metavar "DIRECTION"
                         , help "GPIO pin I/O direction."
                         ]

digitalDirection :: Parser GPIOAction
digitalDirection = GPIODigitalDirection <$> digitalPin <*> direction

analogDirection :: Parser GPIOAction
analogDirection = GPIOAnalogDirection <$> analogPin <*> direction

gpioRead :: Mod CommandFields GPIOAction
gpioRead = command "read" (info (digitalRead <|> analogRead) readI)
    where readI = mconcat [ fullDesc
                          , progDesc "Read a GPIO pin's value."
                          ]

digitalRead :: Parser GPIOAction
digitalRead = GPIODigitalRead <$> digitalPin

analogRead :: Parser GPIOAction
analogRead = GPIOAnalogRead <$> analogPin

gpioWrite :: Mod CommandFields GPIOAction
gpioWrite = command "write" (info (digitalWrite <|> analogWrite) writeI)
    where writeI = mconcat [ fullDesc
                           , progDesc "Set a GPIO pin's state."
                           ]

digitalWrite :: Parser GPIOAction
digitalWrite = GPIODigitalWrite <$> digitalPin <*> bool

analogWrite :: Parser GPIOAction
analogWrite = GPIOAnalogWrite <$> analogPin <*> word16

ledRGB :: Mod CommandFields LEDAction
ledRGB = command "rgb" (info (LEDSetRGB <$> rgb) rgbI)
    where rgbI = mconcat [ fullDesc
                         , progDesc "Set the device's LED RGB state."
                         ]

spiEnable :: Mod CommandFields SPIAction
spiEnable = command "enable" (info (pure SPIEnable) enableI)
    where enableI = mconcat [ fullDesc
                            , progDesc "Enable the SPI bus."
                            ]
spiDisable :: Mod CommandFields SPIAction
spiDisable = command "disable" (info (pure SPIDisable) disableI)
    where disableI = mconcat [ fullDesc
                             , progDesc "Disable the SPI bus."
                             ]

spiRead :: Mod CommandFields SPIAction
spiRead = command "read" (info (pure SPIRead) readI)
    where readI = mconcat [ fullDesc
                          , progDesc "Read a null-terminated string from the SPI bus."
                          ]

spiWriteFromString :: Mod CommandFields SPIAction
spiWriteFromString = command "write" (info (SPIWriteFromString <$> strArgument stringP) writeI)
    where stringP = mconcat [ metavar "PAYLOAD"
                            , help "The string to send over SPI."
                            ]
          writeI = mconcat [ fullDesc
                           , progDesc "Write a null-terminated string to the SPI bus."
                           ]

uartEnable :: Mod CommandFields UARTAction
uartEnable = command "enable" (info (pure UARTEnable) enableI)
    where enableI = mconcat [ fullDesc
                            , progDesc "Enable the UART bus."
                            ]

uartDisable :: Mod CommandFields UARTAction
uartDisable = command "disable" (info (pure UARTDisable) disableI)
    where disableI = mconcat [ fullDesc
                             , progDesc "Disable the UART bus."
                             ]

uartRead :: Mod CommandFields UARTAction
uartRead = command "read" (info (pure UARTRead) readI)
    where readI = mconcat [ fullDesc
                          , progDesc "Read a null-terminated string from the UART bus."
                          ]

uartWriteFromString :: Mod CommandFields UARTAction
uartWriteFromString = command "write" (info (UARTWriteFromString <$> strArgument stringP) writeI)
    where stringP = mconcat [ metavar "PAYLOAD"
                            , help "The string to send over UART."
                            ]
          writeI = mconcat [ fullDesc
                           , progDesc "Write a null-terminated string to the UART bus."
                           ]

moduleID :: Parser ModuleID
moduleID = argument (readParser parseModuleID) moduleP
    where moduleP = mconcat [ metavar "MODULE_ID"
                            , help "The module ID."
                            ]

word8 :: Parser Word8
word8 = argument (readParser parseWord8) mempty

word16 :: Parser Word16
word16 = argument (readParser parseWord16) mempty

bool :: Parser Bool
bool = argument (readParser parseBool) mempty

rgb :: Parser RGB
rgb = RGB <$> argument (readParser parseWord8) rP
          <*> argument (readParser parseWord8) gP
          <*> argument (readParser parseWord8) bP
    where rP = mconcat [ metavar "RED"
                       , help "8-bit red value."
                       ]
          gP = mconcat [ metavar "GREEN"
                       , help "8-bit green value."
                       ]
          bP = mconcat [ metavar "BLUE"
                       , help "8-bit red value."
                       ]
