-> hub
=== hub ===
Hub v1.
* [Plain] -> plain
* [Nested] -> nested
* [Stitch] -> other.inner
* [Cond] -> cond
* [Seq] -> seq
- -> END

=== plain ===
* [P1]
  Plain one v1.
  Plain one second v1.
  -> END
* [P2]
  Plain two v1.
  -> END

=== nested ===
* [N1]
  - - (inner) Nested gather v1.
  ** [NA]
     Nested A v1.
     -> END
  ** [NB]
     Nested B v1.
     -> END
* [N2]
  Nested two v1.
  -> END

=== other ===
= inner
* [S1]
  Stitch one v1.
  -> END
* [S2]
  Stitch two v1.
  -> END

=== cond ===
{ true:
  * [CA]
    Cond A v1.
    -> END
}
{ true:
  * [CB]
    Cond B v1.
    -> END
}
* [CP]
  Cond plain v1.
  -> END

=== seq ===
* [Go]
  {stopping:
    - Seq one v1.
      Seq one second v1.
    - Seq two v1.
  }
  -> END
