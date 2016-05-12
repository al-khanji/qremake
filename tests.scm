(null? '())
(null? '(1))

(list-append (list 1 2 3) (list 4 5))

(list-invert '())
(list-invert '(1))
(list-invert '(1 2))
(list-invert '(1 2 3))

(define long-list '(1 2 3 4 5 6 7 8 "\"hello \"world" "more (text) 'here()() :)"))

(list-invert long-list)

(map null? '(1 2 () 3 4))

(define (is-null? x)
    (if (null? x)
        (list 'yes-null x)
        (list 'not-null x)))

(map is-null? '(1 2 () 3 4))

(list? '())

(map list? '(1 2 () 3 4))

(define (caar x) (car (car x)))
(define (cddr x) (cdr (cdr x)))

(define (cadr x) (car (cdr x)))
(define (caadr x) (caar (cdr x)))
(define (cdar x) (cdr (car x)))

(define (caddr x) (car (cddr x)))
(define (cadddr x) (car (cddr (cdr x))))

(caar '((1) (2) (3)))
(cadr '((1) (2) (3)))
(caadr '((1) (2) (3)))
(caddr '((1) (2) (3)))

(define (skip-last elems)
    (if (null? (cdr elems))
        '()
        (list-append (list (car elems)) (skip-last (cdr elems)))))

(define (get-last elems)
    (if (null? (cdr elems))
        (car elems)
        (get-last (cdr elems))))

(define test-list '((x 1)(y 2)(z 3)(foo)))

(skip-last test-list)
(map car (skip-last test-list))

(get-last test-list)


(define (zip a b)
    (if (null? a)
        a
        (list-append (list (list (car a) (car b))) (zip (cdr a) (cdr b)))))

(zip '(a b c) '(1 2 3))

(define (list-files dir)
    (cadr (system-exec "ls" dir)))

(define new-line-char "
")

(define (ls dir)
    (map print (string-split (list-files dir) new-line-char 'SkipEmptyParts)))

(ls "/")

(map-list (list car cdr) (list (list 1 2 3) (list 4 5 6)))


(define fn-list '(car cdr ls))
(define arg-list '((1 2 3) (4 5 6) "/"))

(zip fn-list arg-list)

(car (zip fn-list arg-list))

(define actions (list car cdr))
(define data (list '(1 2 3) '(4 5 6)))

(define fn-args '(1 2 3 4))

(list fn-args)
(apply list fn-args)

(identity fn-args)
(apply identity (list fn-args))

;(list 1 2 3 4)
;(apply list fn-args)

;(cdr '(1 2 3))
;(apply cdr '((1 2 3)))

(define (list-apply elems) (apply (car elems) (cdr elems)))
(list-apply (list car '(1 2 3)))
(list-apply (list cdr '(1 2 3)))
