-> hub
=== hub ===
Hub v2.
* [Plain] -> plain
* [Nested] -> nested
* [Stitch] -> other.inner
* [Cond] -> cond
* [Seq] -> seq
* [New] -> END
- -> END

=== plain ===
* [P1]
  Plain one v2.
  Plain one second v2.
  -> END
* [P2]
  Plain two v2.
  -> END
* [P3] -> END

=== nested ===
* [N1]
  - - (inner) Nested gather v2.
  ** [NA]
     Nested A v2.
     -> END
  ** [NB]
     Nested B v2.
     -> END
  ** [NC] -> END
* [N2]
  Nested two v2.
  -> END
* [N3] -> END

=== other ===
= inner
* [S1]
  Stitch one v2.
  -> END
* [S2]
  Stitch two v2.
  -> END
* [S3] -> END

=== cond ===
{ true:
  * [CA]
    Cond A v2.
    -> END
}
{ true:
  * [CB]
    Cond B v2.
    -> END
}
* [CP]
  Cond plain v2.
  -> END
* [CN] -> END

=== seq ===
* [Go]
  {stopping:
    - Seq one v2.
      Seq one second v2.
    - Seq two v2.
  }
  -> END
