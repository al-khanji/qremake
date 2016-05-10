(define (null? L) (eq? L '()))
(define (list-append L1 L2) (if (null? L1) L2 (cons (car L1) (list-append (cdr L1) L2)))))
(define (list-invert items) (if (null? items) items (list-append (list-invert (cdr items)) (list (car items)))))
(define (map fn items) (if (null? items) items (list-append (list (fn (car items))) (map fn (cdr items)))))
