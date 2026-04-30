set expandtab

let &path.= "godot-cpp/gen/include,godot-cpp/gdextension/godot-cpp/inlcude,"
let g:ale_c_build_dir_names = ['build-clang-tidy', 'build', 'bin']

let g:ale_fixers = {
    \ 'hpp': ['clang-format'],
    \ 'h': ['clang-format'],
    \ 'c': ['clang-format'],
    \ 'cpp': ['clang-format']
    \}

let g:ale_linters = {
    \ 'h': ['clangtidy'],
    \ 'hpp': ['clangtidy'],
    \ 'c': ['clangtidy'],
    \ 'cpp': ['clangtidy']
    \}

let g:ale_c_clangformat_options = '--style=file'
let g:ale_cpp_clangformat_options = '--style=file'
let g:ale_c_clangtidy_extra_options = '-extra-arg=-Wno-macro-redefined'
let g:ale_cpp_clangtidy_extra_options = '-extra-arg=-Wno-macro-redefined'

set colorcolumn=120
set textwidth=120

autocmd FileType xml setlocal shiftwidth=4 noexpandtab

let g:ale_fix_on_save = 1

function! s:configure_format_on_save() abort
  let b:ale_fix_on_save = expand('%:p') =~# '/src/maszyna/' ? 0 : 1
endfunction

augroup AleFixOnSaveMaszyna
  autocmd!
  autocmd BufRead,BufNewFile *.c,*.h,*.cpp,*.hpp call s:configure_format_on_save()
augroup END
