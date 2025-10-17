import type { TMCPItem } from "@/lib/types";

const WEB_SEARCH_DATA = "webSearchProviderData";

export type WebSearchData = {
  provider: string;
  key: string;
} | null;

class WebSearch {
  private tools: TMCPItem[];

  private webSearchData: WebSearchData = null;

  constructor() {
    this.tools = [];

    const data = localStorage.getItem(WEB_SEARCH_DATA);

    if (data) {
      this.webSearchData = JSON.parse(data);
    } else {
      this.webSearchData = null;
    }

    this.initTools();
  }

  setAllowAlways = () => {
    //skip
  };

  getAllowAlways = () => {
    return true;
  };

  setWebSearchData = (data: WebSearchData) => {
    this.webSearchData = data;
    localStorage.setItem(WEB_SEARCH_DATA, data ? JSON.stringify(data) : "");
    this.initTools();
  };

  getWebSearchData = () => {
    return this.webSearchData;
  };

  setTools = (tools: TMCPItem[]) => {
    this.tools = tools;
  };

  getTools = () => {
    return [...this.tools];
  };

  webSearch = async (args: Record<string, unknown>) => {
    if (this.webSearchData?.provider === "exa") {
      try {
        const result = await new Promise<{
          error?: number;
          data?: unknown;
          message?: string;
        }>((resolve) => {
          window.AscSimpleRequest.createRequest({
            url: "https://api.exa.ai/search",
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "x-api-key": this.webSearchData!.key,
            },
            body: JSON.stringify({
              query: args.query,
              text: true,
              numResults: 5,
              livecrawl: "preferred",
            }),
            complete: function (e: { responseText: string }) {
              const data = JSON.parse(e.responseText).results;
              resolve({ data });
            },
            error: function (e: { statusCode: number }) {
              console.log("Request failed with status:", e.statusCode);
              if (e.statusCode == -102) e.statusCode = 404;
              resolve({
                error: e.statusCode,
                message: `Network error: ${e.statusCode}`,
              });
            },
          });
        });

        return JSON.stringify(result);
      } catch (e) {
        console.error("WebSearch error:", e);
      }
    }
    return JSON.stringify(args);
  };

  webCrawling = async (args: Record<string, unknown>) => {
    if (this.webSearchData?.provider === "exa") {
      try {
        const result = await new Promise<{
          error?: number;
          data?: unknown;
          message?: string;
        }>((resolve) => {
          window.AscSimpleRequest.createRequest({
            url: "https://api.exa.ai/contents",
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "x-api-key": this.webSearchData!.key,
            },
            body: JSON.stringify({
              urls: args.urls,
              text: true,
            }),
            complete: function (e: { responseText: string }) {
              const data = JSON.parse(e.responseText).results;
              resolve({ data });
            },
            error: function (e: { statusCode: number }) {
              console.log("Request failed with status:", e.statusCode);
              if (e.statusCode == -102) e.statusCode = 404;
              resolve({
                error: e.statusCode,
                message: `Network error: ${e.statusCode}`,
              });
            },
          });
        });

        return JSON.stringify(result);
      } catch (e) {
        console.error(e);
      }
    }
    return JSON.stringify(args);
  };

  callTools = async (name: string, args: Record<string, unknown>) => {
    if (name === "web_search") {
      return await this.webSearch(args);
    }

    if (name === "web_crawling") {
      return await this.webCrawling(args);
    }
  };

  initTools = () => {
    if (!this.webSearchData) {
      this.setTools([]);

      return;
    }

    this.setTools([
      {
        name: "web_search",
        description:
          "The search endpoint lets you intelligently search the web and extract contents from the results.",
        inputSchema: {
          query: {
            type: "string",
            description: "The query string for the search.",
          },
        },
      },
      {
        name: "web_crawling",
        description:
          "Get the full page contents, summaries, and metadata for a list of URLs.",
        inputSchema: {
          urls: {
            type: "array",
            items: {
              type: "string",
            },
            description: "Array of URLs to crawl",
          },
        },
      },
    ]);
  };

  getWebSearchEnabled = () => {
    return !!this.webSearchData;
  };
}

export { WebSearch };
