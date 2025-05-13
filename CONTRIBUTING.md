We do rebases here. When merging to master, use gitlab UI. When merging to your branch either do the same or: git switch master; git rebase <your_branch>. If there are any conflicts, resolve them and do git rebase --continue.


Always format code according to .clang-format config in root directory before making commit.
