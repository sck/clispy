#! ./clispy

(define count (lambda (item L) (if L (+ (equal? item (first L)) (count item (rest L))) 0)))
(display (count 0 (list 0 1 2 3 0 0)))
(newline)
