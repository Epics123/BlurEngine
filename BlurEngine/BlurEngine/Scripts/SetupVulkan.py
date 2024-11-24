import os
import sys
import subprocess
from pathlib import Path

import Utils

from io import BytesIO

from urllib.request import urlopen

class VulkanConfiguration:
	requiredVulkanVersion = "1.3.296.0"
	vulkanDirectory = "../VulkanRenderer/Libraries/VulkanSDK"

	@classmethod
	def Validate(cls):
		if (not cls.CheckVulkanSDK()):
			print("Vulkan SDK not installed correctly.")
			return

	@classmethod
	def CheckVulkanSDK(cls):
		vulkanSDK = os.environ.get("VULKAN_SDK")
		if (vulkanSDK is None):
			print("\nYou don't have the Vulkan SDK installed!")
			cls.__InstallVulkanSDK()
			return False
		else:
			print(f"\nLocated Vulkan SDK at {vulkanSDK}")

		if (cls.requiredVulkanVersion not in vulkanSDK):
			print(f"You don't have the correct Vulkan SDK version! (Engine requires {cls.requiredVulkanVersion})")
			cls.__InstallVulkanSDK()
			return False
    
		print(f"Correct Vulkan SDK located at {vulkanSDK}")
		return True

	@classmethod
	def __InstallVulkanSDK(cls):
		permissionGranted = False
		while not permissionGranted:
			reply = str(input("Would you like to install VulkanSDK {0:s}? [Y/N]: ".format(cls.requiredVulkanVersion))).lower().strip()[:1]
			if reply == 'n':
				return
			permissionGranted = (reply == 'y')

		vulkanInstallURL = f"https://sdk.lunarg.com/sdk/download/1.2.189.2/windows/VulkanSDK-1.2.189.2-Installer.exe"
		vulkanPath = f"{cls.vulkanDirectory}/VulkanSDK-{cls.requiredVulkanVersion}-Installer.exe"
		print("Downloading {0:s} to {1:s}".format(vulkanInstallURL, vulkanPath))
		Utils.DownloadFile(vulkanInstallURL, vulkanPath)
		print("Running Vulkan SDK installer...")
		os.startfile(os.path.abspath(vulkanPath))
		print("Re-run this script after installation!")
		quit()


if __name__ == "__main__":
	VulkanConfiguration.Validate()