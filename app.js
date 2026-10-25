document.addEventListener("DOMContentLoaded", () => {
  const promptOutput = document.getElementById("promptOutput");
  const copyPromptBtn = document.getElementById("copyPrompt");
  const consoleStream = document.getElementById("consoleStream");
  const templateButtons = document.querySelectorAll(".template-btn");
  const projectTags = document.querySelectorAll(".tag");
  const projectCards = document.querySelectorAll(".project-card");
  const osOverlay = document.getElementById("osOverlay");
  const closeOsBtn = document.getElementById("closeOs");
  const openOsButtons = document.querySelectorAll("[data-open-os]");
  const systemClock = document.getElementById("systemClock");
  const appTiles = document.querySelectorAll(".app-tile, .dock-app");

  const updateClock = () => {
    const now = new Date();
    systemClock.textContent = now.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
  };

  updateClock();
  setInterval(updateClock, 15000);

  const openOs = () => {
    osOverlay.classList.add("show");
    osOverlay.setAttribute("aria-hidden", "false");
    document.body.style.overflow = "hidden";
  };

  const closeOs = () => {
    osOverlay.classList.remove("show");
    osOverlay.setAttribute("aria-hidden", "true");
    document.body.style.overflow = "";
  };

  openOsButtons.forEach((button) => {
    button.addEventListener("click", openOs);
  });

  closeOsBtn.addEventListener("click", closeOs);

  osOverlay.addEventListener("click", (event) => {
    if (event.target === osOverlay) {
      closeOs();
    }
  });

  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape" && osOverlay.classList.contains("show")) {
      closeOs();
    }
  });

  appTiles.forEach((tile) => {
    tile.addEventListener("click", () => {
      const target = tile.dataset.app;
      document.querySelectorAll(".dock-app").forEach((item) => {
        item.classList.toggle("is-active", item.dataset.app === target);
      });

      const labelMap = {
        browser: "Portfolio Browser",
        terminal: "System Terminal",
        files: "DevPulse Files",
        tools: "Agent Console"
      };

      const title = document.querySelector(".window-label");
      if (title) title.textContent = labelMap[target] || "Portfolio Browser";
    });
  });

  const templates = {
    web: `You are an AI web scraper agent. Extract product data from target listings, structure the output into JSON, and summarize key trends with confidence scoring and anomalies.`,
    code: `You are an AI code optimizer. Review the application logic, remove bottlenecks, improve maintainability, and suggest a cleaner architecture with exact refactoring steps.`,
    bot: `You are an AI automation bot. Create resilient workflow logic, trigger orchestration steps, validate outputs, and generate a concise execution summary for human review.`
  };

  templateButtons.forEach((button) => {
    button.addEventListener("click", () => {
      const key = button.dataset.template;
      promptOutput.value = templates[key];
    });
  });

  copyPromptBtn.addEventListener("click", async () => {
    try {
      await navigator.clipboard.writeText(promptOutput.value);
      copyPromptBtn.textContent = "Copied";
      copyPromptBtn.setAttribute("aria-live", "polite");
      setTimeout(() => {
        copyPromptBtn.textContent = "Copy Prompt";
      }, 1200);
    } catch (error) {
      copyPromptBtn.textContent = "Copy failed";
      setTimeout(() => {
        copyPromptBtn.textContent = "Copy Prompt";
      }, 1200);
    }
  });

  const logMessages = [
    "[system] Initializing DevPulse agent runtime...",
    "[network] Establishing secure connection to model gateway...",
    "[agent] Loading prompt context and vector memory...",
    "[executor] Running workflow orchestration sequence...",
    "[parser] Structuring agent output for downstream tasks...",
    "[signal] Connection successful — model ready for inference.",
    "[tool] Executing script baseline validation...",
    "[ops] Memory cache synced with the latest context window.",
    "[audit] Guardrails active — anomaly detection enabled.",
    "[terminal] Response generated with low-latency execution path.",
    "[agent] Preparing final task summary and command queue...",
    "[done] Deployment prepared for review."
  ];

  let logIndex = 0;

  function addLogLine() {
    const line = document.createElement("div");
    line.className = "log-line my-1 text-xs sm:text-sm";

    const message = logMessages[logIndex % logMessages.length];
    const color = message.includes("[system]") || message.includes("[done]")
      ? "text-emerald-300"
      : message.includes("[network]") || message.includes("[signal]")
      ? "text-cyan-300"
      : message.includes("[tool]") || message.includes("[executor]")
      ? "text-violet-300"
      : "text-slate-200";

    line.classList.add(color);
    line.textContent = message;

    const firstChild = consoleStream.firstChild;
    consoleStream.insertBefore(line, firstChild);

    const lines = consoleStream.querySelectorAll(".log-line");
    if (lines.length > 10) {
      lines[lines.length - 1].remove();
    }

    logIndex += 1;
  }

  setInterval(addLogLine, 1200);

  projectTags.forEach((tag) => {
    tag.addEventListener("click", () => {
      const filter = tag.dataset.filter;

      projectTags.forEach((item) => {
        const isActive = item === tag;
        item.classList.toggle("active", isActive);
        item.setAttribute("aria-pressed", String(isActive));
        item.classList.toggle("border-cyan-400/30", isActive);
        item.classList.toggle("text-cyan-100", isActive);
        item.classList.toggle("bg-cyan-500/10", isActive);
        item.classList.toggle("border-slate-700", !isActive);
        item.classList.toggle("bg-slate-900", !isActive);
        item.classList.toggle("text-slate-200", !isActive);
      });

      projectCards.forEach((card) => {
        const show = filter === "all" || card.dataset.category === filter;
        card.style.display = show ? "block" : "none";
      });
    });
  });
});
