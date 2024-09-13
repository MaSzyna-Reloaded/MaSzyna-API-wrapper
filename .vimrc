set expandtab

let &path.= "godot-cpp/gen/include,godot-cpp/gdextension/godot-cpp/inlcude,"

let g:ale_fixers = {
    \ 'cpp' : ['clang-format'],
    \ 'hpp' : ['clang-format']
    \}

let g:ale_linters = {
    \ 'cpp' : ['clangtidy'],
    \ 'hpp' : ['clangtidy']
    \}

set colorcolumn=120
set textwidth=120

autocmd FileType xml setlocal shiftwidth=4 noexpandtab
