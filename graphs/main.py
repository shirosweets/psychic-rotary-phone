import sys

from os.path import getsize, splitext, basename

from measurement import Measurement
from grapher import graph_measurements


def parse_data(filename: str) -> list[Measurement]:
    measurements = []
    with open(filename, 'r') as f:
        for line in f:
            case = splitext(basename(filename))[0]
            measurements.append(Measurement.parse(line, case))
    return measurements


def main():
    filenames = sys.argv[1:]
    validated_filenames = []
    print(f"Plotting graphics for files: {filenames}")
    for filename in filenames:
        try:
            with open(filename, 'r') as f:
                print(
                    f"File {filename} has "
                    f"{getsize(filename)} bytes in size"
                )
            validated_filenames.append(filename)
        except (IndexError, FileNotFoundError):
            print(f"Skip {filename} - Not found")

    measurements: list[Measurement] = []
    for filename in validated_filenames:
        measurements.extend(parse_data(filename))
    for measurement in measurements:
        print(measurement)
    graph_measurements(measurements)


if __name__ == "__main__":
    main()
