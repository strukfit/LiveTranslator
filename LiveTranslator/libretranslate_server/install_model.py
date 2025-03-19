import sys
import logging
import argostranslate.package
import argostranslate.settings
from pathlib import Path
import shutil

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    stream=sys.stdout 
)

def install_model(from_code, to_code, install_path):
    argostranslate.settings.package_data_dir = Path(install_path)
    argostranslate.settings.downloads_dir = Path(install_path)

    logging.info(f"Updating package index for {from_code} -> {to_code}")
    argostranslate.package.update_package_index()
    
    logging.info("Fetching available packages")
    available_packages = argostranslate.package.get_available_packages()
    try:
        available_package = next(
            x for x in available_packages if x.from_code == from_code and x.to_code == to_code
        )
        logging.info(f"Found package: {available_package.from_code} -> {available_package.to_code}")
    except StopIteration:
        logging.error(f"Package {from_code}-{to_code} not found in available packages")
        print(f"Package {from_code}-{to_code} not found in available packages")
        sys.exit(1)

    logging.info(f"Downloading package {from_code}-{to_code}")
    download_path = available_package.download()

    logging.info(f"Installing package from {download_path}")
    argostranslate.package.install_from_path(download_path)
    
    print(f"Package {from_code}-{to_code} installed successfully")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python install_model.py <from_code> <to_code> <install_path>")
        sys.exit(1)
    
    from_code = sys.argv[1]
    to_code = sys.argv[2]
    install_path = sys.argv[3]
    install_model(from_code, to_code, install_path)