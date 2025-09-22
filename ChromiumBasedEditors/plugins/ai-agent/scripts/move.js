import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Configuration
const sourceDir = path.join(
  __dirname,
  "..",
  "{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}"
);

// Default path - can be overridden via command line argument
const defaultTargetPath = "";

// Get target path from command line argument or use default
const customPath = process.argv[2] || defaultTargetPath;
const targetDir = path.join(
  customPath,
  "{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}"
);

// Function to recursively copy directory
function copyDirectory(src, dest) {
  // Create destination directory if it doesn't exist
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }

  const items = fs.readdirSync(src);

  items.forEach((item) => {
    const srcPath = path.join(src, item);
    const destPath = path.join(dest, item);
    const stat = fs.statSync(srcPath);

    if (stat.isDirectory()) {
      copyDirectory(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
    }
  });
}

// Function to remove directory recursively
function removeDirectory(dirPath) {
  if (fs.existsSync(dirPath)) {
    fs.rmSync(dirPath, { recursive: true, force: true });
    console.log(`Removed existing directory: ${dirPath}`);
  }
}

// Check if source directory exists
if (!fs.existsSync(sourceDir)) {
  console.error(
    "❌ Error: Source plugin directory not found. Please run build first."
  );
  console.error(`Expected: ${sourceDir}`);
  process.exit(1);
}

// Ensure target parent directory exists
const targetParent = path.dirname(targetDir);
if (!fs.existsSync(targetParent)) {
  console.log(`Creating target directory: ${targetParent}`);
  fs.mkdirSync(targetParent, { recursive: true });
}

console.log("🚀 Starting plugin move process...");
console.log(`Source: ${sourceDir}`);
console.log(`Target: ${targetDir}`);

try {
  // Remove existing target directory if it exists
  if (fs.existsSync(targetDir)) {
    console.log("\n Removing existing plugin directory...");
    removeDirectory(targetDir);
  }

  // Copy plugin directory to target location
  console.log("\n📁 Copying plugin directory...");
  copyDirectory(sourceDir, targetDir);

  console.log("\n✅ Plugin moved successfully!");
  console.log(`Plugin is now available at: ${targetDir}`);
} catch (error) {
  console.error("❌ Error during move process:", error);
  process.exit(1);
}
