const LESSONS = {
  1: {
    title: "Assignment 1 - Variables and Assignment",
    objective: "Understand var/const declarations and safe reassignment.",
    explanation: [
      "In MagPhos, variables are explicit. You declare mutable values with var and stable values with const.",
      "Assignment updates an existing name; it should not silently create a new one. That keeps code predictable.",
      "For this assignment, you will declare hp, declare maxHp, and then decrease hp once.",
      "Think of this as your base game state update pattern: create state, then mutate intentionally."
    ],
    starter: "# Assignment 1\n# Declare hp and maxHp, then update hp\n",
    task: "Write: var hp = 100, const maxHp = 100, then hp = hp - 15.",
    check: (code) => /\bvar\s+hp\s*=\s*100\b/.test(code)
      && /\bconst\s+maxHp\s*=\s*100\b/.test(code)
      && /\bhp\s*=\s*hp\s*-\s*15\b/.test(code),
    terminalHint: "Expected behavior: hp starts at 100, then becomes 85."
  },
  2: {
    title: "Assignment 2 - Functions and Return",
    objective: "Model reusable logic with fn and return values.",
    explanation: [
      "Functions package repeated logic. In MagPhos, a function starts with fn and can take parameters.",
      "return sends a value back to the caller. Keep return inside functions only.",
      "For gameplay scripting, functions keep combat and scoring logic clean and testable.",
      "In this task, you will define a function and call it to produce a result."
    ],
    starter: "# Assignment 2\n# Build add(a, b) and call it\n",
    task: "Write fn add(a, b) { return a + b } and then result = add(4, 6).",
    check: (code) => /\bfn\s+add\s*\(\s*a\s*,\s*b\s*\)/.test(code)
      && /\breturn\s+a\s*\+\s*b\b/.test(code)
      && /\bresult\s*=\s*add\s*\(\s*4\s*,\s*6\s*\)/.test(code),
    terminalHint: "Expected behavior: result should hold 10."
  },
  3: {
    title: "Assignment 3 - if / else Flow",
    objective: "Branch behavior based on runtime state.",
    explanation: [
      "Branching decides what happens when conditions change.",
      "if runs when condition is true; else is fallback behavior.",
      "Game scripting relies on branch clarity for win/lose states, dialogue routes, and AI decisions.",
      "Your job is to branch on hp and print two different outcomes."
    ],
    starter: "# Assignment 3\nvar hp = 0\n# Add if/else here\n",
    task: "Write if hp <= 0 { print \"dead\" } else { print \"alive\" }.",
    check: (code) => /\bif\s+hp\s*<=\s*0\s*\{/.test(code)
      && /\belse\s*\{/.test(code)
      && /print\s+"dead"/.test(code)
      && /print\s+"alive"/.test(code),
    terminalHint: "Expected behavior: with hp = 0, output should be dead."
  },
  4: {
    title: "Assignment 4 - Loop Fundamentals",
    objective: "Repeat logic deterministically with for loops.",
    explanation: [
      "Loops let you repeat behavior with predictable updates.",
      "A for loop has initializer, condition, and increment. Keep increments explicit.",
      "This pattern is useful for wave spawns, tick updates, and batched processing in game scripts.",
      "In this assignment, print i from 0 through 2."
    ],
    starter: "# Assignment 4\n# Print i in a for loop\n",
    task: "Write for (var i = 0; i < 3; i = i + 1) { print i }.",
    check: (code) => /\bfor\s*\(\s*var\s+i\s*=\s*0\s*;\s*i\s*<\s*3\s*;\s*i\s*=\s*i\s*\+\s*1\s*\)\s*\{/.test(code)
      && /\bprint\s+i\b/.test(code),
    terminalHint: "Expected behavior: output lines 0, 1, 2."
  },
  5: {
    title: "Assignment 5 - switch / default",
    objective: "Handle discrete states clearly.",
    explanation: [
      "switch is ideal when one variable maps to different behaviors.",
      "case handles a specific value; default handles everything else.",
      "This is common for mode handling, state machines, and menu flow control.",
      "You will create a switch statement with one case and a default fallback."
    ],
    starter: "# Assignment 5\nvar state = \"idle\"\n# Add switch here\n",
    task: "Write switch state with case \"idle\" and a default block.",
    check: (code) => /\bswitch\s+state\s*\{/.test(code)
      && /\bcase\s+"idle"\s*\{/.test(code)
      && /\bdefault\s*\{/.test(code),
    terminalHint: "Expected behavior: idle path prints your case message."
  },
  6: {
    title: "Assignment 6 - Hard Practice: fix broken script",
    objective: "Repair common syntax and logic mistakes before final unlock.",
    explanation: [
      "This page simulates real debugging. Broken starter code contains three bugs.",
      "You must fix declaration safety, function return usage, and braces in control flow.",
      "Once this passes, you have completed the guided assignment path.",
      "After passing, continue to playground and create your own mini game loop from scratch."
    ],
    starter: "# Assignment 6 (broken on purpose)\nscore = 10\nfn gain(x) {\n  x + 5\n}\nif score > 0\n  print gain(score)\n",
    task: "Fix all issues: declare score, return in gain, and add braces for if block.",
    check: (code) => /\bvar\s+score\s*=\s*10\b/.test(code)
      && /\bfn\s+gain\s*\(\s*x\s*\)\s*\{[\s\S]*\breturn\s+x\s*\+\s*5\b/.test(code)
      && /\bif\s+score\s*>\s*0\s*\{[\s\S]*print\s+gain\s*\(\s*score\s*\)/.test(code),
    terminalHint: "Expected behavior: script should be structurally valid and print updated value path."
  }
};

function getCompleted() {
  return Number(localStorage.getItem("magphos_training_completed") || 0);
}

function setCompleted(value) {
  localStorage.setItem("magphos_training_completed", String(value));
}

function renderHub() {
  const list = document.getElementById("lessonList");
  const progress = document.getElementById("hubProgress");
  if (!list || !progress) return;

  const completed = getCompleted();
  progress.textContent = `Progress: ${completed}/6 assignments passed`;

  list.innerHTML = "";
  Object.keys(LESSONS).map(Number).forEach((id) => {
    const lesson = LESSONS[id];
    const unlocked = id <= completed + 1;
    const passed = id <= completed;
    const item = document.createElement("li");
    item.className = "lesson-item";
    item.innerHTML = `
      <h3>${lesson.title}</h3>
      <p class="muted">${lesson.objective}</p>
      <div class="lesson-actions"></div>
    `;
    const actions = item.querySelector(".lesson-actions");
    const a = document.createElement("a");
    a.className = "link-btn";
    a.textContent = passed ? "Review" : (unlocked ? "Start" : "Locked");
    a.href = unlocked ? `training/lesson${id}.html` : "#";
    if (!unlocked) {
      a.style.opacity = "0.5";
      a.style.pointerEvents = "none";
    }
    actions.appendChild(a);
    list.appendChild(item);
  });
}

function renderLessonPage() {
  const root = document.getElementById("lessonRoot");
  if (!root) return;
  const id = Number(root.dataset.lesson || 0);
  const lesson = LESSONS[id];
  if (!lesson) return;

  const completed = getCompleted();
  if (id > completed + 1) {
    window.location.href = "../training.html";
    return;
  }

  const titleEl = document.getElementById("lessonTitle");
  const objectiveEl = document.getElementById("lessonObjective");
  const taskEl = document.getElementById("lessonTask");
  const explainEl = document.getElementById("lessonExplanation");
  const editorEl = document.getElementById("lessonEditor");
  const terminalEl = document.getElementById("lessonTerminal");
  const statusEl = document.getElementById("lessonStatus");
  const nextBtn = document.getElementById("nextLessonBtn");
  const progressEl = document.getElementById("lessonProgress");

  titleEl.textContent = lesson.title;
  objectiveEl.textContent = lesson.objective;
  taskEl.textContent = lesson.task;
  explainEl.innerHTML = lesson.explanation.map((line) => `<p>${line}</p>`).join("");
  editorEl.value = lesson.starter;
  terminalEl.textContent = "Terminal ready. Click Run Check after writing your code.";

  progressEl.textContent = `Progress: ${completed}/6 assignments passed`;

  function markPassed() {
    const currentCompleted = getCompleted();
    if (id > currentCompleted) {
      setCompleted(id);
    }
    statusEl.innerHTML = `<span class="status-ok">PASS</span> Assignment cleared.`;
    terminalEl.textContent = `[PASS]\n${lesson.terminalHint}\n\nYour current solution:\n${editorEl.value}`;

    if (id < 6) {
      nextBtn.disabled = false;
      nextBtn.textContent = `Go to Assignment ${id + 1}`;
      nextBtn.onclick = () => {
        window.location.href = `lesson${id + 1}.html`;
      };
    } else {
      nextBtn.disabled = false;
      nextBtn.textContent = "Finish and return to Academy";
      nextBtn.onclick = () => {
        window.location.href = "../training.html";
      };
    }

    progressEl.textContent = `Progress: ${getCompleted()}/6 assignments passed`;
  }

  function markFail() {
    statusEl.innerHTML = `<span class="status-bad">NOT YET</span> Fix the task requirements and try again.`;
    terminalEl.textContent = `[CHECK FAILED]\n${lesson.terminalHint}\n\nReview your code and match the task exactly.`;
  }

  document.getElementById("loadStarterBtn").addEventListener("click", () => {
    editorEl.value = lesson.starter;
    terminalEl.textContent = "Starter loaded. Make your edits, then Run Check.";
  });

  document.getElementById("runCheckBtn").addEventListener("click", () => {
    const passed = lesson.check(editorEl.value);
    if (passed) {
      markPassed();
    } else {
      markFail();
    }
  });

  nextBtn.disabled = true;
}

window.addEventListener("DOMContentLoaded", () => {
  renderHub();
  renderLessonPage();
});
