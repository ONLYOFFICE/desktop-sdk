import { StrictMode } from "react";
import { createRoot } from "react-dom/client";

import App from "./App.tsx";
import "./index.css";

const initializeApp = () => {
  // Try to find root element in current document
  let rootElement = document.getElementById("root");

  // If not found, try to find it in the iframe
  if (!rootElement) {
    const iframe = window.parent.document.querySelector(
      'iframe[name="system_asc.{9DC93CDB-B576-4F0C-B55E-FCC9C48DD777}"]'
    ) as HTMLIFrameElement;
    if (iframe && iframe.contentDocument) {
      rootElement = iframe.contentDocument.getElementById("root");
    }
  }

  // If still not found, create the root element in the current document body
  if (!rootElement) {
    rootElement = document.createElement("div");
    rootElement.id = "root";
    document.body.appendChild(rootElement);
  }

  // Check if root element hasn't been initialized
  if (rootElement && !rootElement.hasAttribute("data-react-root")) {
    rootElement.setAttribute("data-react-root", "true");
    createRoot(rootElement).render(
      <StrictMode>
        <App />
      </StrictMode>
    );
  }
};

// Initialize when DOM is ready
if (document.readyState === "loading") {
  document.addEventListener("DOMContentLoaded", initializeApp);
} else {
  initializeApp();
}
