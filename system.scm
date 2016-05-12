(define (null? L) (eq? L '()))

(define (identity x) x)

(define (list-append L1 L2)
    (if (null? L1)
        L2
        (cons (car L1) (list-append (cdr L1) L2))))

(define (map-list fns items)
    (if (null? items)
        items
        (list-append (list ((car fns) (list (car items)))) (map-list (cdr fns) (cdr items)))))

(define (map-fn fn items)
    (if (null? items)
        items
        (list-append (list (fn (car items))) (map-fn fn (cdr items)))))

(define (map f items)
    (if (callable? f)
        (map-fn f items)
        (map-list f items)))

(define (list-invert items)
    (if (null? items)
        items
        (list-append (list-invert (cdr items)) (list (car items)))))


