import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const distDir = path.join(__dirname, "..", "dist");
const targetDir = path.join(
  __dirname,
  "..",
  "{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}"
);

// Ensure target directory exists
if (!fs.existsSync(targetDir)) {
  fs.mkdirSync(targetDir, { recursive: true });
}

// Function to clean target directory while preserving specific files
function cleanTargetDir(dirPath) {
  const preserveFiles = ["index.html", "config.json"];

  if (!fs.existsSync(dirPath)) {
    return;
  }

  const items = fs.readdirSync(dirPath);

  items.forEach((item) => {
    if (preserveFiles.includes(item)) {
      console.log(`Preserving: ${item}`);
      return;
    }

    const itemPath = path.join(dirPath, item);
    const stat = fs.statSync(itemPath);

    if (stat.isDirectory()) {
      // Remove directory recursively
      fs.rmSync(itemPath, { recursive: true, force: true });
      console.log(`Removed directory: ${item}`);
    } else {
      // Remove file
      fs.unlinkSync(itemPath);
      console.log(`Removed file: ${item}`);
    }
  });
}

// Function to copy files recursively
function copyFiles(srcDir, destDir) {
  const items = fs.readdirSync(srcDir);

  items.forEach((item) => {
    const srcPath = path.join(srcDir, item);
    const destPath = path.join(destDir, item);

    // Skip index.html
    if (item === "index.html") {
      console.log(`Skipping: ${item}`);
      return;
    }

    const stat = fs.statSync(srcPath);

    if (stat.isDirectory()) {
      // Create directory if it doesn't exist
      if (!fs.existsSync(destPath)) {
        fs.mkdirSync(destPath, { recursive: true });
      }
      // Recursively copy directory contents
      copyFiles(srcPath, destPath);
    } else {
      // Copy file
      fs.copyFileSync(srcPath, destPath);
    }
  });
}

// Check if dist directory exists
if (!fs.existsSync(distDir)) {
  console.error("Error: dist directory not found. Please run build first.");
  process.exit(1);
}

try {
  // Clean target directory while preserving index.html and config.json
  console.log("\n🧹 Cleaning target directory...");
  cleanTargetDir(targetDir);

  // Copy files from dist
  console.log("\n📁 Copying files...");
  copyFiles(distDir, targetDir);

  console.log("\n✅ Build script completed successfully!");
} catch (error) {
  console.error("❌ Error during build process:", error);
  process.exit(1);
}
