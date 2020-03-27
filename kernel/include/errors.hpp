#ifndef FREELSD_KERNEL_ERRORS_HEADER
#define FREELSD_KERNEL_ERRORS_HEADER

#ifdef __cplusplus
extern "C" {
#endif

const char errfrog[] = "                             .-----.\n                            /7  .  (\n                           /   .-.  \\\n                          /   /   \\  \\\n                         / `  )   (   )\n                        / `   )   ).  \\\n                      .\'  _.   \\_/  . |\n     .--.           .\' _.\' )`.        |\n    (    `---...._.\'   `---.\'_)    ..  \\\n     \\            `----....___    `. \\  |\n      `.           _ ----- _   `._  )/  |\n        `.       /\"  \\   /\"  \\`.  `._   |\n          `.    ((O)` ) ((O)` ) `.   `._\\\n            `-- \'`---\'   `---\' )  `.    `-.\n               /                  ` \\      `-.\n             .\'                      `.       `.\n            /                     `  ` `.       `-.\n     .--.   \\ ===._____.======. `    `   `. .___.--`     .\'\'\'\'.\n    \' .` `-. `.                )`. `   ` ` \\          .\' . \'  8)\n   (8  .  ` `-.`.               ( .  ` `  .`\\      .\'  \'    \' /\n    \\  `. `    `-.               ) ` .   ` ` \\  .\'   \' .  \'  /\n     \\ ` `.  ` . \\`.    .--.     |  ` ) `   .``/   \'  // .  /\n      `.  ``. .   \\ \\   .-- `.  (  ` /_   ` . / \' .  \'/   .\'\n        `. ` \\  `  \\ \\  \'-.   `-\'  .\'  `-.  `   .  .\'/  .\'\n          \\ `.`.  ` \\ \\    ) /`._.`       `.  ` .  .\'  /\n           |  `.`. . \\ \\  (.\'               `.   .\'  .\'\n        __/  .. \\ \\ ` ) \\                     \\.\' .. \\__\n .-._.-\'     \'\"  ) .-\'   `.                   (  \'\"     `-._.--.\n(_________.-====\' / .\' /\\_)`--..__________..-- `====-. _________)\n\n";
const char stdfrog[] = "\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  kermit says guten tag\n \\/   \\/\n\n";

// Halt kernel execution and display a panic screen.
__attribute__((noreturn)) void panic(const char *message);

#ifdef __cplusplus
}
#endif

#endif
