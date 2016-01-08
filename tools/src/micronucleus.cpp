#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

#include <micronucleus_util.h>
#include <delay_util.h>

using namespace std;

enum filetype_t {INTEL_HEX, RAW};

#define CONNECT_WAIT 250 /* milliseconds to wait after detecting device on usb bus - probably excessive */


static const char * const usage = "usage: micronucleus [--help] [--run] [--dump-progress] [--fast-mode] "
    "[--type intel-hex|raw] [--timeout integer] [--erase-only] filename";

static const char * const prefix[] = {"micronucleus: ", "              "};

static const char * const finishedString = "micronucleus done.  Thank you.";

static const char * const problemString = "Problem uploading to board.";


// Intel HEX parsing helper function
static unsigned char parseHex(FILE *input) {
  char buf[3] = {getc(input), getc(input), 0};
  return strtol(buf, NULL, 16);
}

static void printProgress(float f) {
  static int last = 0;
  
  int current = (int)(f * 50);

  if (current < last) {
    last = 0;
  }

  while (current > last) {
    cout << "#";
    last++;
  }
}


int main(int argc, char **argv) {
  struct {
    const char *filename;
    filetype_t filetype;
    bool run, dumpProgress, eraseOnly, fastMode;
    int timeout;
  } options = {NULL, INTEL_HEX, false, false, false, false, 0};
  
  unsigned char dataBuffer[65536 + 256];

  int step = 0, totalSteps = 5; // default: waiting, connecting, parsing, erasing, writing

  int res; // return result from erasing, writing and running

  if (argc < 2) {
    cout << usage;
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      cout << usage << endl
           << endl
           << "  --type [intel-hex, raw]: Set upload file type to either intel hex or raw"    << endl
           << "                           bytes (intel hex is default)."                      << endl
           << "          --dump-progress: Output progress data in computer-friendly form"     << endl
           << "                           for driving GUIs."                                  << endl
           << "             --erase-only: Erase the device without programming. Fills the"    << endl
           << "                           program memory with 0xFFFF. Any files are ignored." << endl
           << "              --fast-mode: Speed up the timing of micronucleus. Do not use if" << endl
           << "                           you encounter USB errors."                          << endl
           << "                    --run: Ask bootloader to run the program when finished"    << endl
           << "                           uploading provided program."                        << endl
           << "      --timeout [integer]: Timeout after waiting specified number of seconds." << endl
           << "                 filename: Path to intel hex or raw data file to upload,"      << endl
           << "                           or \"-\" to read from stdin."                       << endl
           << endl
           << "Commandline tool version " << MICRONUCLEUS_COMMANDLINE_VERSION;

      exit(EXIT_SUCCESS);
    } else if (strcmp(argv[i], "--run") == 0) {
      options.run = true;
      totalSteps += 1;
    } else if (strcmp(argv[i], "--dump-progress") == 0) {
      options.dumpProgress = true;
    } else if (strcmp(argv[i], "--fast-mode") == 0) {
      options.fastMode = true;
    } else if (strcmp(argv[i], "--type") == 0) {
      i++;
      if (strcmp(argv[i], "raw") == 0) {
        options.filetype = RAW;
      } else if (strcmp(argv[i], "intel-hex") != 0) {
        cout << prefix[0] << "Unknown File Type specified with --type option.";
        exit(EXIT_FAILURE);
      }
    } else if (strcmp(argv[i], "--timeout") == 0) {
      i++;
      if ((options.timeout = strtol(argv[i], NULL, 10)) == 0) {
        cout << prefix[0] << "Did not understand timeout value \"" << argv[i] << "\".";
        exit(EXIT_FAILURE);
      }
    } else if (strcmp(argv[i], "--erase-only") == 0) {
      options.eraseOnly = true;
      totalSteps -= 1;
    } else {
      options.filename = argv[i];
    }
  }

  cout << prefix[0] << "Commandline tool version " << MICRONUCLEUS_COMMANDLINE_VERSION << endl
       << endl
       << prefix[0] << "Please connect the device." << endl
       << prefix[0] << "Searching for device... ";

  time_t startTime, currentTime;
  time(&startTime);

  Micronucleus micronucleus;
  micronucleus.callbackFn = printProgress;

  do {
    micronucleus.connect(options.fastMode);
    time(&currentTime);
  } while (!micronucleus.connected && (options.timeout == 0 || currentTime < startTime + options.timeout));

  if (!micronucleus.connected) {
    cout << "Failed!" << endl << prefix[0] << "Search timed out." << endl;
    exit(EXIT_FAILURE);
  }

  cout << "OK!" << endl; // device found

  if (!options.fastMode) {
    cout << prefix[0] << "Waiting for device... ";
    delay(CONNECT_WAIT);
    cout << "OK!" << endl;
  }

  cout << endl
       << prefix[0] << "Device firmware   : Version " << (int)micronucleus.version.major << "." << (int)micronucleus.version.minor << endl
       << prefix[1] << "Device signature  : 0x1E" << hex << uppercase << setfill('0') << setw(4) << micronucleus.signature << dec << nouppercase << setfill(' ') << setw(0) << endl
       << prefix[1] << "Available space   : " << micronucleus.flashSize << " bytes" << endl
       << prefix[1] << "Write sleep time  : " << micronucleus.writeSleep << "ms" << endl
       << prefix[1] << "Erase sleep time  : " << micronucleus.eraseSleep << "ms" << endl
       << prefix[1] << "Page count        : " << micronucleus.pages << endl
       << prefix[1] << "Page size         : " << micronucleus.pageSize << endl
       << endl;

  unsigned int startAddress = 0, endAddress = 0;

  if (!options.eraseOnly) {
    FILE *input;

    if (strcmp(options.filename, "-") == 0) {
      input = stdin;
      cout << prefix[0] << "Reading from stdin." << endl;
    } else {
      input = fopen(options.filename, "r");
      cout << prefix[0] << "Reading input file \"" << options.filename << "\"." << endl;
    }

    if (!input) {
      cout << prefix[0] << "Error reading file: " << strerror(errno);
      exit(EXIT_FAILURE);
    }

    switch (options.filetype) {
      case INTEL_HEX: {
        unsigned char byteCount, address, recordType;

        do {
          while (getc(input) != ':') continue;

          unsigned char sum = 0;
          
          sum += (byteCount  = parseHex(input));
          sum += (address = (parseHex(input) << 8) | parseHex(input));
          sum += (recordType = parseHex(input));

          switch (recordType) {
            case 0x00: {
              // data
              for (unsigned int i = address; i < address + byteCount; i++) {
                sum += (dataBuffer[i] = parseHex(input));
              }
            }

            case 0x01: {
              // end of file -- will be handled later
              break;
            }

            default: {
              // unknown
              cout << prefix[0] << "Unknown record type in Intel HEX file. Exiting." << endl << problemString;
              exit(EXIT_FAILURE);
            }
          }

          // add checksum
          sum += parseHex(input);

          if (sum != 0) {
            cout << hex << uppercase
                 << prefix[0] << "Warning: Checksum error between addresses 0x" << base << " and 0x" << address << "." << endl
                 << dec << nouppercase;
          }

          if (startAddress > address) startAddress = address;
          if (endAddress < address + byteCount) endAddress = address + byteCount;

        } while (recordType != 0x01);

        break; // end case INTEL_HEX
      }

      case RAW: {
        for (int c; (c = getc(input)) != EOF; dataBuffer[endAddress++] = c) continue;
        break;
      }
    }

    fclose(input);

    if (startAddress == endAddress) {
      cout << prefix[0] << "Input file is empty. Exiting." << endl << problemString;
      exit(EXIT_FAILURE);
    }

    if (endAddress > micronucleus.flashSize) {
      cout << prefix[0] << "Input file is too large (" << endAddress << " > " << micronucleus.flashSize << ")." << endl << problemString;
      exit(EXIT_FAILURE);
    }
  }

  cout << endl << "Erasing | ";
  res = micronucleus.erase();
  cout << " | 100%" << endl << endl;

  switch (res) {
    case 0: {
      // erase successful; no need to do anything
      break;
    }
    
    case 1: {
      // error: device disconnected during erase
      if (!options.eraseOnly || options.run) { // no need to reconnect if there's nothing more to do
        cout << prefix[0] << "Connection to device lost during erase." << endl
             << prefix[0] << "Attempting to reconnect... ";

        delay(CONNECT_WAIT);

        int timeout = 5; // alert after 5 seconds
        time(&startTime);

        do {
          micronucleus.connect(options.fastMode);
          time(&currentTime);

          if (timeout > 0 && currentTime >= startTime + timeout) {
            cout << "Failed!" << endl
                 << prefix[0] << "Automatic reconnect timed out. Please unplug" << endl
                 << prefix[1] << "the device and reconnect manually." << endl
                 << prefix[0] << "Waiting for reconnect... ";

            timeout = 0;
          }
        } while (!micronucleus.connected);

        cout << "OK!" << endl << endl;
      }
      break;
    }
    
    default: {
      cout << prefix[0] << "Flash error " << res << " has occurred." << endl
           << prefix[0] << "Please unplug the device and try again." << endl << problemString;

      exit(EXIT_FAILURE);
      break;
    }
  }

  if (!options.eraseOnly) {
    cout << "Writing | ";
    res = micronucleus.write(dataBuffer, endAddress);
    cout << " | 100%" << endl << endl;

    if (res != 0) {
      cout << prefix[0] << "Write error " << res << " has occurred." << endl
           << prefix[0] << "Please unplug the device and try again." << endl << problemString;

      exit(EXIT_FAILURE);
    }
  }

  if (options.run) {
    cout << prefix[0] << "Starting the user app... ";

    res = micronucleus.run();

    if (res != 0) {
      cout << prefix[0] << "Run error " << res << " has occurred." << endl
           << prefix[0] << "Please unplug the device and try again." << endl << problemString;

      exit(EXIT_FAILURE);
    } else {
      cout << "OK!" << endl;
    }
  }

  cout << endl << endl << finishedString << endl;

  exit(EXIT_SUCCESS);
}