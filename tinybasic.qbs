Application {
  Depends {
    name: "cpp"
  }
  cpp.includePaths: ["./include"]
  cpp.cLanguageVersion: "gnu2x"
  name: "tinybasic"
  cpp.optimization: "fast"
  files: [
    "src/main.c",
    "src/basic.c",
    "src/basic_mem.c",
    "src/basic_parse.c",
  ]
}
